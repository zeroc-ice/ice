// Copyright (c) ZeroC, Inc.

#include "../../src/Ice/SSL/AppleSSLUtil.h"
#include "../../src/Ice/UniqueRef.h"
#include "Ice/SSL/ClientAuthenticationOptions.h"
#include "Ice/SSL/ServerAuthenticationOptions.h"
#include "Test.h"
#include "TestHelper.h"
#include "TestI.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <thread>

using namespace std;
using namespace Ice;
using namespace Ice::SSL;
using namespace Test;

#ifdef ICE_USE_APPLE_SSL

atomic<int> nextKeyChain = 1000;

string
getKeyChainPath(const string& basePath)
{
    ostringstream os;
    os << basePath << "/keychain/" << nextKeyChain++ << ".keychain";
    return os.str();
}

#    ifdef ICE_USE_APPLE_SSL_IOS
string
getResourcePath(const string& path)
{
    CFBundleRef bundle = CFBundleGetMainBundle();
    if (bundle)
    {
        IceInternal::UniqueRef<CFStringRef> resourceName(toCFString(path));
        IceInternal::UniqueRef<CFURLRef> url(CFBundleCopyResourceURL(bundle, resourceName.get(), nullptr, nullptr));

        UInt8 filePath[PATH_MAX];
        if (CFURLGetFileSystemRepresentation(url.get(), true, filePath, sizeof(filePath)))
        {
            return string(reinterpret_cast<char*>(filePath));
        }
    }
    return "";
}
#    endif

const char* const password = "password";
const char* const keychainPassword = "password";

Ice::CommunicatorPtr
createServer(ServerAuthenticationOptions serverAuthenticationOptions, TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = initialize();

    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints(
        "ServerAdapter",
        helper->getTestEndpoint(10, "ssl"),
        serverAuthenticationOptions);
    auto obj = adapter->add<ServerPrx>(make_shared<ServerI>(communicator), Identity{.name = "server"});
    adapter->activate();
    return communicator;
}

Ice::CommunicatorPtr
createClient(const optional<ClientAuthenticationOptions>& clientAuthenticationOptions = nullopt)
{
    Ice::InitializationData initData{
        .clientAuthenticationOptions = clientAuthenticationOptions.value_or(ClientAuthenticationOptions{})};

    if (IceInternal::isMinBuild())
    {
        initData.pluginFactories = {Ice::wsPluginFactory()};
    }

    return initialize(std::move(initData));
}

void
clientValidatesServerSettingTrustedRootCertificates(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "client validates server certificate setting trusted root certificates... " << flush;
    CFArrayRef serverCertificateChain = Apple::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef trustedRootCertificates = Apple::loadCACertificates(certificatesPath + "/ca1/ca1_cert.pem");
    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverCertificateSelectionCallback = [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
            .trustedRootCertificates = trustedRootCertificates};
        Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
        obj->ice_ping();
    }
    catch (...)
    {
        CFRelease(serverCertificateChain);
        CFRelease(trustedRootCertificates);
        throw;
    }
    CFRelease(serverCertificateChain);
    CFRelease(trustedRootCertificates);
    cout << "ok" << endl;
}

void
clientValidatesServerUsingValidationCallback(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "client validates server certificate using validation callback... " << flush;
    CFArrayRef serverCertificateChain = Apple::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    // The server certificate is not trusted by the client CA, but the validation callback accepts the server
    // certificate.
    CFArrayRef trustedRootCertificates = Apple::loadCACertificates(certificatesPath + "/ca2/ca2_cert.pem");
    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverCertificateSelectionCallback = [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
            .trustedRootCertificates = trustedRootCertificates,
            .serverCertificateValidationCallback = [](SecTrustRef, const Ice::SSL::ConnectionInfoPtr&)
            { return true; }};
        Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
        obj->ice_ping();
    }
    catch (...)
    {
        CFRelease(serverCertificateChain);
        CFRelease(trustedRootCertificates);
        throw;
    }
    CFRelease(serverCertificateChain);
    CFRelease(trustedRootCertificates);
    cout << "ok" << endl;
}

void
clientRejectsServerSettingTrustedRootCertificates(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "client rejects server certificate setting trusted root certificates... " << flush;
    CFArrayRef serverCertificateChain = Apple::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef trustedRootCertificates = Apple::loadCACertificates(certificatesPath + "/ca2/ca2_cert.pem");
    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverCertificateSelectionCallback = [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
            .trustedRootCertificates = trustedRootCertificates};
        Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
        try
        {
            obj->ice_ping();
            test(false);
        }
        catch (const Ice::SecurityException&)
        {
            // Expected
        }
    }
    catch (...)
    {
        CFRelease(serverCertificateChain);
        CFRelease(trustedRootCertificates);
        throw;
    }
    CFRelease(serverCertificateChain);
    CFRelease(trustedRootCertificates);
    cout << "ok" << endl;
}

void
clientRejectsServerUsingDefaultTrustedRootCertificates(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "client rejects server certificate using default trusted root certificates... " << flush;
    CFArrayRef serverCertificateChain = Apple::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverCertificateSelectionCallback = [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::CommunicatorHolder clientCommunicator(createClient());

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
        try
        {
            obj->ice_ping();
            test(false);
        }
        catch (const Ice::SecurityException&)
        {
            // Expected
        }
    }
    catch (...)
    {
        CFRelease(serverCertificateChain);
        throw;
    }
    CFRelease(serverCertificateChain);
    cout << "ok" << endl;
}

void
clientRejectsServerUsingValidationCallback(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "client rejects server certificate using validation callback... " << flush;
    CFArrayRef serverCertificateChain = Apple::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    // The client trusted root certificates include the server certificate CA, but the validation callback
    // rejects the server certificate.
    CFArrayRef trustedRootCertificates = Apple::loadCACertificates(certificatesPath + "/ca1/ca1_cert.pem");
    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverCertificateSelectionCallback = [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
            .trustedRootCertificates = trustedRootCertificates,
            .serverCertificateValidationCallback = [](SecTrustRef, const Ice::SSL::ConnectionInfoPtr&)
            { return false; }};
        Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
        try
        {
            obj->ice_ping();
            test(false);
        }
        catch (const Ice::SecurityException&)
        {
            // Expected
        }
    }
    catch (...)
    {
        CFRelease(serverCertificateChain);
        CFRelease(trustedRootCertificates);
        throw;
    }
    CFRelease(serverCertificateChain);
    CFRelease(trustedRootCertificates);
    cout << "ok" << endl;
}

void
serverValidatesClientSettingTrustedRootCertificates(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "server validates client certificate setting trusted root certificates... " << flush;
    CFArrayRef serverCertificateChain = Apple::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef clientCertificateChain = Apple::loadCertificateChain(
        certificatesPath + "/ca1/client.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    // The client certificate is trusted by the server CA.
    CFArrayRef trustedRootCertificates = Apple::loadCACertificates(certificatesPath + "/ca1/ca1_cert.pem");
    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverCertificateSelectionCallback =
                [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            },
            .clientCertificateRequired = true,
            .trustedRootCertificates = trustedRootCertificates};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
            .clientCertificateSelectionCallback =
                [clientCertificateChain](const string&)
            {
                CFRetain(clientCertificateChain);
                return clientCertificateChain;
            },
            .trustedRootCertificates = trustedRootCertificates};
        Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
        obj->ice_ping();
    }
    catch (...)
    {
        CFRelease(serverCertificateChain);
        CFRelease(clientCertificateChain);
        CFRelease(trustedRootCertificates);
        throw;
    }
    CFRelease(serverCertificateChain);
    CFRelease(clientCertificateChain);
    CFRelease(trustedRootCertificates);
    cout << "ok" << endl;
}

void
serverValidatesClientUsingValidationCallback(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "server validates client certificate using validation callback... " << flush;
    CFArrayRef serverCertificateChain = Apple::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    // The client certificate is not trusted by the server CA, but the validation callback accepts the client
    // certificate.
    CFArrayRef serverRootCertificates = Apple::loadCACertificates(certificatesPath + "/ca2/ca2_cert.pem");

    CFArrayRef clientCertificateChain = Apple::loadCertificateChain(
        certificatesPath + "/ca1/client.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef clientRootCertificates = Apple::loadCACertificates(certificatesPath + "/ca1/ca1_cert.pem");
    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverCertificateSelectionCallback =
                [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            },
            .clientCertificateRequired = true,
            .trustedRootCertificates = serverRootCertificates,
            .clientCertificateValidationCallback = [](SecTrustRef, const Ice::SSL::ConnectionInfoPtr&)
            { return true; }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
            .clientCertificateSelectionCallback =
                [clientCertificateChain](const string&)
            {
                CFRetain(clientCertificateChain);
                return clientCertificateChain;
            },
            .trustedRootCertificates = clientRootCertificates};
        Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
        obj->ice_ping();
    }
    catch (...)
    {
        CFRelease(serverCertificateChain);
        CFRelease(clientCertificateChain);
        CFRelease(clientRootCertificates);
        CFRelease(serverRootCertificates);
        throw;
    }
    CFRelease(serverCertificateChain);
    CFRelease(clientCertificateChain);
    CFRelease(clientRootCertificates);
    CFRelease(serverRootCertificates);
    cout << "ok" << endl;
}

void
serverRejectsClientSettingTrustedRootCertificates(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "server rejects client certificate setting trusted root certificates... " << flush;
    CFArrayRef serverCertificateChain = Apple::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef clientCertificateChain = Apple::loadCertificateChain(
        certificatesPath + "/ca1/client.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef clientRootCertificates = Apple::loadCACertificates(certificatesPath + "/ca1/ca1_cert.pem");
    CFArrayRef serverRootCertificates = Apple::loadCACertificates(certificatesPath + "/ca2/ca2_cert.pem");
    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverCertificateSelectionCallback =
                [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            },
            .clientCertificateRequired = true,
            .trustedRootCertificates = serverRootCertificates};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
            .clientCertificateSelectionCallback =
                [clientCertificateChain](const string&)
            {
                CFRetain(clientCertificateChain);
                return clientCertificateChain;
            },
            .trustedRootCertificates = clientRootCertificates};
        Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
        try
        {
            obj->ice_ping();
            test(false);
        }
        catch (const Ice::ConnectionLostException&)
        {
            // Expected
        }
    }
    catch (...)
    {
        CFRelease(serverCertificateChain);
        CFRelease(clientCertificateChain);
        CFRelease(clientRootCertificates);
        CFRelease(serverRootCertificates);
        throw;
    }
    CFRelease(serverCertificateChain);
    CFRelease(clientCertificateChain);
    CFRelease(clientRootCertificates);
    CFRelease(serverRootCertificates);
    cout << "ok" << endl;
}

void
serverRejectsClientUsingDefaultTrustedRootCertificates(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "server rejects client certificate using default root certificates... " << flush;
    CFArrayRef serverCertificateChain = Apple::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef clientCertificateChain = Apple::loadCertificateChain(
        certificatesPath + "/ca1/client.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef trustedRootCertificates = Apple::loadCACertificates(certificatesPath + "/ca1/ca1_cert.pem");
    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverCertificateSelectionCallback =
                [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            },
            .clientCertificateRequired = true};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
            .clientCertificateSelectionCallback =
                [clientCertificateChain](const string&)
            {
                CFRetain(clientCertificateChain);
                return clientCertificateChain;
            },
            .trustedRootCertificates = trustedRootCertificates};
        Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
        try
        {
            obj->ice_ping();
            test(false);
        }
        catch (const Ice::ConnectionLostException&)
        {
            // Expected
        }
    }
    catch (...)
    {
        CFRelease(serverCertificateChain);
        CFRelease(clientCertificateChain);
        CFRelease(trustedRootCertificates);
        throw;
    }
    CFRelease(serverCertificateChain);
    CFRelease(clientCertificateChain);
    CFRelease(trustedRootCertificates);
    cout << "ok" << endl;
}

void
serverRejectsClientUsingValidationCallback(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "server rejects client certificate using validation callback... " << flush;
    CFArrayRef serverCertificateChain = Apple::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef clientCertificateChain = Apple::loadCertificateChain(
        certificatesPath + "/ca1/client.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef trustedRootCertificates = Apple::loadCACertificates(certificatesPath + "/ca1/ca1_cert.pem");
    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverCertificateSelectionCallback =
                [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            },
            .clientCertificateRequired = true,
            .trustedRootCertificates = trustedRootCertificates,
            .clientCertificateValidationCallback = [](SecTrustRef, const Ice::SSL::ConnectionInfoPtr&)
            { return false; }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
            .clientCertificateSelectionCallback =
                [clientCertificateChain](const string&)
            {
                CFRetain(clientCertificateChain);
                return clientCertificateChain;
            },
            .trustedRootCertificates = trustedRootCertificates};
        Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
        try
        {
            obj->ice_ping();
            test(false);
        }
        catch (const Ice::ConnectionLostException&)
        {
            // Expected
        }
    }
    catch (...)
    {
        CFRelease(serverCertificateChain);
        CFRelease(clientCertificateChain);
        CFRelease(trustedRootCertificates);
        throw;
    }
    CFRelease(serverCertificateChain);
    CFRelease(clientCertificateChain);
    CFRelease(trustedRootCertificates);
    cout << "ok" << endl;
}

void
newSessionCallbacksAreInvoked(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "client and server new session callbacks are invoked... " << flush;
    CFArrayRef serverCertificateChain = Apple::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef trustedRootCertificates = Apple::loadCACertificates(certificatesPath + "/ca1/ca1_cert.pem");
    try
    {
        int serverCalls = 0;
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverCertificateSelectionCallback =
                [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            },
            .sslNewSessionCallback =
                [&serverCalls](sec_protocol_options_t secOptions, const string& adapterName)
            {
                test(secOptions);
                test(adapterName == "ServerAdapter");
                sec_protocol_options_set_min_tls_protocol_version(secOptions, tls_protocol_version_TLSv12);
                ++serverCalls;
            }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        int clientCalls = 0;
        Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
            .sslNewSessionCallback =
                [&clientCalls](sec_protocol_options_t secOptions, const string&)
            {
                test(secOptions);
                sec_protocol_options_set_min_tls_protocol_version(secOptions, tls_protocol_version_TLSv12);
                ++clientCalls;
            },
            .trustedRootCertificates = trustedRootCertificates};
        Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
        obj->ice_ping();

        // The server callback runs when the listener is created, the client callback once per connection.
        test(serverCalls == 1);
        test(clientCalls == 1);
    }
    catch (...)
    {
        CFRelease(serverCertificateChain);
        CFRelease(trustedRootCertificates);
        throw;
    }
    CFRelease(serverCertificateChain);
    CFRelease(trustedRootCertificates);
    cout << "ok" << endl;
}

void
serverCertificateSelectionCallbackFailure(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "server certificate selection callback failure... " << flush;
    CFArrayRef serverCertificateChain = Apple::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef trustedRootCertificates = Apple::loadCACertificates(certificatesPath + "/ca1/ca1_cert.pem");
    try
    {
        // The callback runs per handshake, from a dispatch thread: an exception must fail the handshake, not
        // the server.
        atomic<bool> fail{true};
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverCertificateSelectionCallback = [&fail, serverCertificateChain](const string&)
            {
                if (fail)
                {
                    throw Ice::SecurityException(__FILE__, __LINE__, "certificate reload failed");
                }
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        {
            Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
                .trustedRootCertificates = trustedRootCertificates};
            Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

            ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
            try
            {
                obj->ice_ping();
                test(false);
            }
            catch (const Ice::ConnectionLostException&)
            {
                // Expected: the server has no certificate for the handshake.
            }
            catch (const Ice::SecurityException&)
            {
                // Also acceptable, depending on how the failed handshake is reported.
            }
        }

        // The server is still running and serves connections once the callback provides the certificate.
        fail = false;
        {
            Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
                .trustedRootCertificates = trustedRootCertificates};
            Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

            ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
            obj->ice_ping();
        }
    }
    catch (...)
    {
        CFRelease(serverCertificateChain);
        CFRelease(trustedRootCertificates);
        throw;
    }
    CFRelease(serverCertificateChain);
    CFRelease(trustedRootCertificates);
    cout << "ok" << endl;
}

void
certificateSelectionCallbackReturnsCertificateOnly(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "certificate selection callback returning a certificate instead of an identity... " << flush;
    CFArrayRef serverCertificateChain = Apple::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef trustedRootCertificates = Apple::loadCACertificates(certificatesPath + "/ca1/ca1_cert.pem");
    // A chain without an identity: certificates only, as SecItemImport returns for a certificate whose key is
    // already in the keychain. It must be rejected, never passed to Network.framework as the identity.
    CFArrayRef certificateOnlyChain = Apple::loadCACertificates(certificatesPath + "/ca1/ca1_cert.pem");
    test(CFGetTypeID(CFArrayGetValueAtIndex(certificateOnlyChain, 0)) == SecCertificateGetTypeID());
    try
    {
        // Server side: the handshake fails and the server keeps running.
        atomic<bool> certificateOnly{true};
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverCertificateSelectionCallback =
                [&certificateOnly, serverCertificateChain, certificateOnlyChain](const string&)
            {
                CFArrayRef chain = certificateOnly ? certificateOnlyChain : serverCertificateChain;
                CFRetain(chain);
                return chain;
            }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        {
            Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
                .trustedRootCertificates = trustedRootCertificates};
            Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

            ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
            try
            {
                obj->ice_ping();
                test(false);
            }
            catch (const Ice::ConnectionLostException&)
            {
                // Expected: the server has no identity for the handshake.
            }
            catch (const Ice::SecurityException&)
            {
                // Also acceptable, depending on how the failed handshake is reported.
            }
        }

        certificateOnly = false;

        // Client side: the connection fails with SecurityException.
        {
            Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
                .clientCertificateSelectionCallback =
                    [certificateOnlyChain](const string&)
                {
                    CFRetain(certificateOnlyChain);
                    return certificateOnlyChain;
                },
                .trustedRootCertificates = trustedRootCertificates};
            Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

            ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
            try
            {
                obj->ice_ping();
                test(false);
            }
            catch (const Ice::SecurityException&)
            {
                // Expected
            }
        }

        // The server serves connections once its callback returns an identity.
        {
            Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
                .trustedRootCertificates = trustedRootCertificates};
            Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

            ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
            obj->ice_ping();
        }
    }
    catch (...)
    {
        CFRelease(serverCertificateChain);
        CFRelease(trustedRootCertificates);
        CFRelease(certificateOnlyChain);
        throw;
    }
    CFRelease(serverCertificateChain);
    CFRelease(trustedRootCertificates);
    CFRelease(certificateOnlyChain);
    cout << "ok" << endl;
}

void
configurationExceptionsArePreserved(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "client and server TLS configuration exceptions are preserved... " << flush;
    CFArrayRef serverCertificateChain = Apple::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef trustedRootCertificates = Apple::loadCACertificates(certificatesPath + "/ca1/ca1_cert.pem");
    try
    {
        // Network.framework invokes the TLS configuration synchronously when the listener or connection parameters
        // are created: an exception thrown by an application callback there must not escape through
        // Network.framework, it is preserved and rethrown by the object adapter creation (server) or the
        // connection establishment (client).
        {
            Ice::CommunicatorHolder serverCommunicator(initialize());
            Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
                .serverCertificateSelectionCallback =
                    [serverCertificateChain](const string&)
                {
                    CFRetain(serverCertificateChain);
                    return serverCertificateChain;
                },
                .sslNewSessionCallback = [](sec_protocol_options_t, const string&)
                { throw Ice::SecurityException(__FILE__, __LINE__, "server configuration failure"); }};
            try
            {
                serverCommunicator->createObjectAdapterWithEndpoints(
                    "ServerAdapter",
                    helper->getTestEndpoint(10, "ssl"),
                    serverAuthenticationOptions);
                test(false);
            }
            catch (const Ice::SecurityException& ex)
            {
                test(string(ex.what()).find("server configuration failure") != string::npos);
            }
        }

        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverCertificateSelectionCallback = [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        for (const string& failure : {"client identity selection failure", "client configuration failure"})
        {
            Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
                .trustedRootCertificates = trustedRootCertificates};
            if (failure == "client identity selection failure")
            {
                clientAuthenticationOptions.clientCertificateSelectionCallback = [failure](const string&) -> CFArrayRef
                { throw Ice::SecurityException(__FILE__, __LINE__, failure); };
            }
            else
            {
                clientAuthenticationOptions.sslNewSessionCallback = [failure](sec_protocol_options_t, const string&)
                { throw Ice::SecurityException(__FILE__, __LINE__, failure); };
            }
            Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

            ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
            try
            {
                obj->ice_ping();
                test(false);
            }
            catch (const Ice::SecurityException& ex)
            {
                test(string(ex.what()).find(failure) != string::npos);
            }
        }
    }
    catch (...)
    {
        CFRelease(serverCertificateChain);
        CFRelease(trustedRootCertificates);
        throw;
    }
    CFRelease(serverCertificateChain);
    CFRelease(trustedRootCertificates);
    cout << "ok" << endl;
}

void
trustedRootCertificatesAreRetained(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "trusted root certificates are retained by the TLS configuration... " << flush;
    CFArrayRef serverCertificateChain = Apple::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef clientCertificateChain = Apple::loadCertificateChain(
        certificatesPath + "/ca1/client.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef trustedRootCertificates = Apple::loadCACertificates(certificatesPath + "/ca1/ca1_cert.pem");
    const CFIndex baseline = CFGetRetainCount(trustedRootCertificates);

    // Waits until the retain count of the roots satisfies the predicate, for at most the given duration.
    auto waitForRetainCount = [trustedRootCertificates](auto predicate, chrono::seconds timeout)
    {
        auto deadline = chrono::steady_clock::now() + timeout;
        while (!predicate(CFGetRetainCount(trustedRootCertificates)) && chrono::steady_clock::now() < deadline)
        {
            this_thread::sleep_for(chrono::milliseconds(25));
        }
        return predicate(CFGetRetainCount(trustedRootCertificates));
    };

    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverCertificateSelectionCallback =
                [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            },
            .clientCertificateRequired = true,
            .trustedRootCertificates = trustedRootCertificates};
        Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
            .clientCertificateSelectionCallback =
                [clientCertificateChain](const string&)
            {
                CFRetain(clientCertificateChain);
                return clientCertificateChain;
            },
            .trustedRootCertificates = trustedRootCertificates};

        {
            Ice::CommunicatorHolder serverCommunicator(initialize());

            // An object adapter retains the roots of its authentication options; measure that reference with an
            // adapter that has no SSL endpoint, and therefore no TLS listener.
            serverCommunicator->createObjectAdapterWithEndpoints(
                "TcpAdapter",
                helper->getTestEndpoint(11, "tcp"),
                serverAuthenticationOptions);
            const CFIndex adapterReference = CFGetRetainCount(trustedRootCertificates) - baseline;

            // The SSL adapter's listener configures TLS with blocks Network.framework invokes during the
            // handshakes: its TLS configuration retains the roots for them, on top of the adapter's reference.
            ObjectAdapterPtr adapter = serverCommunicator->createObjectAdapterWithEndpoints(
                "ServerAdapter",
                helper->getTestEndpoint(10, "ssl"),
                serverAuthenticationOptions);
            adapter->add(make_shared<ServerI>(serverCommunicator.communicator()), Identity{.name = "server"});
            adapter->activate();
            test(CFGetRetainCount(trustedRootCertificates) > baseline + 2 * adapterReference);
            const CFIndex serverCount = CFGetRetainCount(trustedRootCertificates);

            {
                // The communicator retains the roots of its client authentication options; the connection's TLS
                // configuration retains them as well once the connection is established.
                Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));
                const CFIndex clientCount = CFGetRetainCount(trustedRootCertificates);

                ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
                obj->ice_ping();
                test(CFGetRetainCount(trustedRootCertificates) > clientCount);
            }
            // The connection's TLS configuration is released with the connection. Network.framework tears the
            // TLS state of the closed connections down some seconds after they are released.
            test(
                waitForRetainCount([serverCount](CFIndex count) { return count == serverCount; }, chrono::seconds(60)));
        }
        // The adapters' references and the listener's TLS configuration are released with the communicator.
        test(waitForRetainCount([baseline](CFIndex count) { return count == baseline; }, chrono::seconds(60)));
    }
    catch (...)
    {
        CFRelease(serverCertificateChain);
        CFRelease(clientCertificateChain);
        CFRelease(trustedRootCertificates);
        throw;
    }
    CFRelease(serverCertificateChain);
    CFRelease(clientCertificateChain);
    CFRelease(trustedRootCertificates);
    cout << "ok" << endl;
}

void
plainEndpointsWithServerAuthenticationOptions(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "plain endpoints of an object adapter with server authentication options... " << flush;
    CFArrayRef serverCertificateChain = Apple::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    try
    {
        // The options only apply to the adapter's SSL endpoints: its tcp and ws endpoints accept plain connections.
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverCertificateSelectionCallback = [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            }};
        Ice::InitializationData initData;
        if (IceInternal::isMinBuild())
        {
            initData.pluginFactories = {Ice::wsPluginFactory()};
        }
        Ice::CommunicatorHolder serverCommunicator(initialize(std::move(initData)));
        ObjectAdapterPtr adapter = serverCommunicator->createObjectAdapterWithEndpoints(
            "PlainAdapter",
            helper->getTestEndpoint(12, "tcp") + ":" + helper->getTestEndpoint(13, "ws"),
            serverAuthenticationOptions);
        adapter->add(make_shared<ServerI>(serverCommunicator.communicator()), Identity{.name = "server"});
        adapter->activate();

        Ice::CommunicatorHolder clientCommunicator(createClient());
        for (const string& protocol : {"tcp", "ws"})
        {
            ServerPrx obj(
                clientCommunicator.communicator(),
                "server:" + helper->getTestEndpoint(protocol == "tcp" ? 12 : 13, protocol));
            obj->ice_ping();
        }
    }
    catch (...)
    {
        CFRelease(serverCertificateChain);
        throw;
    }
    CFRelease(serverCertificateChain);
    cout << "ok" << endl;
}

void
serverHotCertificateReload(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "server hot certificate reload... " << flush;
    class ServerState final
    {
    public:
        ServerState(const string& certificatePath, const string& keyChainPath)
            : _serverCertificateChain(
                  Apple::loadCertificateChain(certificatePath, "", keyChainPath, keychainPassword, password))
        {
        }

        ~ServerState()
        {
            if (_serverCertificateChain)
            {
                CFRelease(_serverCertificateChain);
            }
        }

        [[nodiscard]] CFArrayRef serverCertificateChain() const { return _serverCertificateChain; }

        void reloadCertificate(const string& certificatePath, const string& keyChainPath)
        {
            if (_serverCertificateChain)
            {
                CFRelease(_serverCertificateChain);
            }
            _serverCertificateChain =
                Apple::loadCertificateChain(certificatePath, "", keyChainPath, keychainPassword, password);
        }

    private:
        CFArrayRef _serverCertificateChain = nullptr;
    };

    ServerState serverState(certificatesPath + "/ca1/server.p12", getKeyChainPath(certificatesPath));

    CFArrayRef trustedRootCertificatesCA1 = Apple::loadCACertificates(certificatesPath + "/ca1/ca1_cert.pem");
    CFArrayRef trustedRootCertificatesCA2 = Apple::loadCACertificates(certificatesPath + "/ca2/ca2_cert.pem");
    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverCertificateSelectionCallback = [&serverState](const string&)
            {
                CFArrayRef certificateChain = serverState.serverCertificateChain();
                CFRetain(certificateChain);
                return certificateChain;
            }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        {
            Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
                .trustedRootCertificates = trustedRootCertificatesCA1};
            Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

            ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
            obj->ice_ping();
        }

        {
            // CA2 is not accepted with the initial configuration
            Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
                .trustedRootCertificates = trustedRootCertificatesCA2};
            Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

            ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
            try
            {
                obj->ice_ping();
                test(false);
            }
            catch (const Ice::SecurityException&)
            {
                // Expected
            }
        }

        serverState.reloadCertificate(certificatesPath + "/ca2/server.p12", getKeyChainPath(certificatesPath));

        {
            // CA2 is accepted with the new configuration
            Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
                .trustedRootCertificates = trustedRootCertificatesCA2};
            Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

            ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
            obj->ice_ping();
        }

        {
            // CA1 is not accepted after reloading configuration
            Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
                .trustedRootCertificates = trustedRootCertificatesCA1};
            Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

            ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
            try
            {
                obj->ice_ping();
                test(false);
            }
            catch (const Ice::SecurityException&)
            {
                // Expected
            }
        }
    }
    catch (...)
    {
        CFRelease(trustedRootCertificatesCA1);
        CFRelease(trustedRootCertificatesCA2);
        throw;
    }
    CFRelease(trustedRootCertificatesCA1);
    CFRelease(trustedRootCertificatesCA2);
    cout << "ok" << endl;
}

#    ifdef ICE_USE_APPLE_SSL_IOS
void
allAuthenticationOptionsTests(Test::TestHelper* helper, const string&)
{
    const string certificatesPath = getResourcePath("certs/configuration");
#    else
void
allAuthenticationOptionsTests(Test::TestHelper* helper, const string& defaultDir)
{
    const string certificatesPath = defaultDir;
#    endif

    cout << "testing with Apple SSL native APIs..." << endl;

    clientValidatesServerSettingTrustedRootCertificates(helper, certificatesPath);
    clientValidatesServerUsingValidationCallback(helper, certificatesPath);
    clientRejectsServerSettingTrustedRootCertificates(helper, certificatesPath);
    clientRejectsServerUsingDefaultTrustedRootCertificates(helper, certificatesPath);
    clientRejectsServerUsingValidationCallback(helper, certificatesPath);

    serverValidatesClientSettingTrustedRootCertificates(helper, certificatesPath);
    serverValidatesClientUsingValidationCallback(helper, certificatesPath);
    serverRejectsClientSettingTrustedRootCertificates(helper, certificatesPath);
    serverRejectsClientUsingDefaultTrustedRootCertificates(helper, certificatesPath);
    serverRejectsClientUsingValidationCallback(helper, certificatesPath);

    newSessionCallbacksAreInvoked(helper, certificatesPath);

    serverHotCertificateReload(helper, certificatesPath);
    serverCertificateSelectionCallbackFailure(helper, certificatesPath);
    certificateSelectionCallbackReturnsCertificateOnly(helper, certificatesPath);
    configurationExceptionsArePreserved(helper, certificatesPath);
    trustedRootCertificatesAreRetained(helper, certificatesPath);
    plainEndpointsWithServerAuthenticationOptions(helper, certificatesPath);
}
#endif
