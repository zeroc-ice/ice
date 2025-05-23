// Copyright (c) ZeroC, Inc.

#include "../../src/Ice/SSL/SecureTransportUtil.h"
#include "../../src/Ice/UniqueRef.h"
#include "Ice/SSL/ClientAuthenticationOptions.h"
#include "Ice/SSL/ServerAuthenticationOptions.h"
#include "Test.h"
#include "TestHelper.h"
#include "TestI.h"

#include <memory>
#include <string>

using namespace std;
using namespace Ice;
using namespace Ice::SSL;
using namespace Test;

#ifdef ICE_USE_SECURE_TRANSPORT

atomic<int> nextKeyChain = 1000;

string
getKeyChainPath(const string& basePath)
{
    ostringstream os;
    os << basePath << "/keychain/" << nextKeyChain++ << ".keychain";
    return os.str();
}

#    ifdef ICE_USE_SECURE_TRANSPORT_IOS
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
    CFArrayRef serverCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef trustedRootCertificates = SecureTransport::loadCACertificates(certificatesPath + "/ca1/ca1_cert.pem");
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
    CFArrayRef serverCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    // The server certificate is not trusted by the client CA, but the validation callback accepts the server
    // certificate.
    CFArrayRef trustedRootCertificates = SecureTransport::loadCACertificates(certificatesPath + "/ca2/ca2_cert.pem");
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
clientValidatesServerUsingSystemTrustedRootCertificates(Test::TestHelper*, const string&)
{
    cout << "client validates server using system trusted root certificates... " << flush;
    Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{};
    Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

    Ice::ObjectPrx obj(
        clientCommunicator.communicator(),
        "Glacier2/router:wss -p 443 -h zeroc.com -r /demo-proxy/chat/glacier2");
    obj->ice_ping();
    cout << "ok" << endl;
}

void
clientRejectsServerSettingTrustedRootCertificates(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "client rejects server certificate setting trusted root certificates... " << flush;
    CFArrayRef serverCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef trustedRootCertificates = SecureTransport::loadCACertificates(certificatesPath + "/ca2/ca2_cert.pem");
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
    CFArrayRef serverCertificateChain = SecureTransport::loadCertificateChain(
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
    CFArrayRef serverCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    // The client trusted root certificates include the server certificate CA, but the validation callback
    // rejects the server certificate.
    CFArrayRef trustedRootCertificates = SecureTransport::loadCACertificates(certificatesPath + "/ca1/ca1_cert.pem");
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
    CFArrayRef serverCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef clientCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/ca1/client.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    // The client certificate is trusted by the server CA.
    CFArrayRef trustedRootCertificates = SecureTransport::loadCACertificates(certificatesPath + "/ca1/ca1_cert.pem");
    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverCertificateSelectionCallback =
                [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            },
            .clientCertificateRequired = kAlwaysAuthenticate,
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
    CFArrayRef serverCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    // The client certificate is not trusted by the server CA, but the validation callback accepts the client
    // certificate.
    CFArrayRef serverRootCertificates = SecureTransport::loadCACertificates(certificatesPath + "/ca2/ca2_cert.pem");

    CFArrayRef clientCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/ca1/client.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef clientRootCertificates = SecureTransport::loadCACertificates(certificatesPath + "/ca1/ca1_cert.pem");
    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverCertificateSelectionCallback =
                [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            },
            .clientCertificateRequired = kAlwaysAuthenticate,
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
    CFArrayRef serverCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef clientCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/ca1/client.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef clientRootCertificates = SecureTransport::loadCACertificates(certificatesPath + "/ca1/ca1_cert.pem");
    CFArrayRef serverRootCertificates = SecureTransport::loadCACertificates(certificatesPath + "/ca2/ca2_cert.pem");
    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverCertificateSelectionCallback =
                [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            },
            .clientCertificateRequired = kAlwaysAuthenticate,
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
    CFArrayRef serverCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef clientCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/ca1/client.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef trustedRootCertificates = SecureTransport::loadCACertificates(certificatesPath + "/ca1/ca1_cert.pem");
    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverCertificateSelectionCallback =
                [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            },
            .clientCertificateRequired = kAlwaysAuthenticate};
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
    CFArrayRef serverCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/ca1/server.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef clientCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/ca1/client.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef trustedRootCertificates = SecureTransport::loadCACertificates(certificatesPath + "/ca1/ca1_cert.pem");
    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverCertificateSelectionCallback =
                [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            },
            .clientCertificateRequired = kAlwaysAuthenticate,
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
serverHotCertificateReload(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "server hot certificate reload... " << flush;
    class ServerState final
    {
    public:
        ServerState(const string& certificatePath, const string& keyChainPath)
            : _serverCertificateChain(
                  SecureTransport::loadCertificateChain(certificatePath, "", keyChainPath, keychainPassword, password))
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
                SecureTransport::loadCertificateChain(certificatePath, "", keyChainPath, keychainPassword, password);
        }

    private:
        CFArrayRef _serverCertificateChain = nullptr;
    };

    ServerState serverState(certificatesPath + "/ca1/server.p12", getKeyChainPath(certificatesPath));

    CFArrayRef trustedRootCertificatesCA1 = SecureTransport::loadCACertificates(certificatesPath + "/ca1/ca1_cert.pem");
    CFArrayRef trustedRootCertificatesCA2 = SecureTransport::loadCACertificates(certificatesPath + "/ca2/ca2_cert.pem");
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

#    ifdef ICE_USE_SECURE_TRANSPORT_IOS
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

    cout << "testing with SecureTransport native APIs..." << endl;

    clientValidatesServerSettingTrustedRootCertificates(helper, certificatesPath);
    clientValidatesServerUsingValidationCallback(helper, certificatesPath);
    clientValidatesServerUsingSystemTrustedRootCertificates(helper, certificatesPath);
    clientRejectsServerSettingTrustedRootCertificates(helper, certificatesPath);
    clientRejectsServerUsingDefaultTrustedRootCertificates(helper, certificatesPath);
    clientRejectsServerUsingValidationCallback(helper, certificatesPath);

    serverValidatesClientSettingTrustedRootCertificates(helper, certificatesPath);
    serverValidatesClientUsingValidationCallback(helper, certificatesPath);
    serverRejectsClientSettingTrustedRootCertificates(helper, certificatesPath);
    serverRejectsClientUsingDefaultTrustedRootCertificates(helper, certificatesPath);
    serverRejectsClientUsingValidationCallback(helper, certificatesPath);

    serverHotCertificateReload(helper, certificatesPath);
}
#endif
