//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "../../src/Ice/SSL/SecureTransportUtil.h"
#include "Ice/SSL/ClientAuthenticationOptions.h"
#include "Ice/SSL/ServerAuthenticationOptions.h"
#include "Ice/UniqueRef.h"
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

const string password = "password";
const string keychainPassword = "password";

Ice::CommunicatorPtr
createServer(ServerAuthenticationOptions serverAuthenticationOptions, TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = initialize();
    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints(
        "ServerAdapter",
        helper->getTestEndpoint(10, "ssl"),
        serverAuthenticationOptions);
    ServerPrx obj(adapter->add(make_shared<ServerI>(communicator), Identity{.name = "server"}));
    adapter->activate();
    return communicator;
}

void
clientValidatesServerSettingTrustedRootCertificates(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "client validates server setting trusted root certificates... " << flush;
    CFArrayRef serverCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/s_rsa_ca1.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef trustedRootCertificates = SecureTransport::loadCACertificates(certificatesPath + "/cacert1.pem");
    try
    {
        auto serverAuthenticationOptions = Ice::SSL::ServerAuthenticationOptions{
            .serverCertificateSelectionCallback = [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::CommunicatorHolder clientCommunicator(initialize(Ice::InitializationData{
            .clientAuthenticationOptions =
                Ice::SSL::ClientAuthenticationOptions{.trustedRootCertificates = trustedRootCertificates}}));

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
    cout << "client rejects server setting trusted root certificates... " << flush;
    CFArrayRef serverCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/s_rsa_ca1.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef trustedRootCertificates = SecureTransport::loadCACertificates(certificatesPath + "/cacert2.pem");
    try
    {
        auto serverAuthenticationOptions = Ice::SSL::ServerAuthenticationOptions{
            .serverCertificateSelectionCallback = [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::CommunicatorHolder clientCommunicator(initialize(Ice::InitializationData{
            .clientAuthenticationOptions =
                Ice::SSL::ClientAuthenticationOptions{.trustedRootCertificates = trustedRootCertificates}}));

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
        obj->ice_ping();
        test(false);
    }
    catch (const Ice::SecurityException&)
    {
        // Expected
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
    cout << "client rejects server using default trusted root certificates... " << flush;
    CFArrayRef serverCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/s_rsa_ca1.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef trustedRootCertificates = SecureTransport::loadCACertificates(certificatesPath + "/cacert2.pem");
    try
    {
        auto serverAuthenticationOptions = Ice::SSL::ServerAuthenticationOptions{
            .serverCertificateSelectionCallback = [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::CommunicatorHolder clientCommunicator(initialize());

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
        obj->ice_ping();
        test(false);
    }
    catch (const Ice::SecurityException&)
    {
        // Expected
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
clientRejectsServerUsingValidationCallback(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "client rejects server using validation callback... " << flush;
    CFArrayRef serverCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/s_rsa_ca1.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef trustedRootCertificates = SecureTransport::loadCACertificates(certificatesPath + "/cacert2.pem");
    try
    {
        auto serverAuthenticationOptions = Ice::SSL::ServerAuthenticationOptions{
            .serverCertificateSelectionCallback = [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::CommunicatorHolder clientCommunicator(initialize(Ice::InitializationData{
            .clientAuthenticationOptions = Ice::SSL::ClientAuthenticationOptions{
                .trustedRootCertificates = trustedRootCertificates,
                .serverCertificateValidationCallback = [](SecTrustRef, const Ice::SSL::ConnectionInfoPtr&)
                { return false; }}}));

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
        obj->ice_ping();
        test(false);
    }
    catch (const Ice::SecurityException&)
    {
        // Expected
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
    cout << "server validates client setting trusted root certificates... " << flush;
    CFArrayRef serverCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/s_rsa_ca1.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef clientCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/c_rsa_ca1.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef trustedRootCertificates = SecureTransport::loadCACertificates(certificatesPath + "/cacert1.pem");
    try
    {
        auto serverAuthenticationOptions = Ice::SSL::ServerAuthenticationOptions{
            .serverCertificateSelectionCallback =
                [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            },
            .clientCertificateRequired = kAlwaysAuthenticate,
            .trustedRootCertificates = trustedRootCertificates};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::CommunicatorHolder clientCommunicator(initialize(Ice::InitializationData{
            .clientAuthenticationOptions = Ice::SSL::ClientAuthenticationOptions{
                .clientCertificateSelectionCallback =
                    [clientCertificateChain](const string&)
                {
                    CFRetain(clientCertificateChain);
                    return clientCertificateChain;
                },
                .trustedRootCertificates = trustedRootCertificates}}));

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
serverRejectsClientSettingTrustedRootCertificates(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "server rejects client setting trusted root certificates... " << flush;
    CFArrayRef serverCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/s_rsa_ca1.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef clientCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/c_rsa_ca1.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef trustedRootCertificates1 = SecureTransport::loadCACertificates(certificatesPath + "/cacert1.pem");
    CFArrayRef trustedRootCertificates2 = SecureTransport::loadCACertificates(certificatesPath + "/cacert2.pem");
    try
    {
        auto serverAuthenticationOptions = Ice::SSL::ServerAuthenticationOptions{
            .serverCertificateSelectionCallback =
                [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            },
            .clientCertificateRequired = kAlwaysAuthenticate,
            .trustedRootCertificates = trustedRootCertificates2};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::CommunicatorHolder clientCommunicator(initialize(Ice::InitializationData{
            .clientAuthenticationOptions = Ice::SSL::ClientAuthenticationOptions{
                .clientCertificateSelectionCallback =
                    [clientCertificateChain](const string&)
                {
                    CFRetain(clientCertificateChain);
                    return clientCertificateChain;
                },
                .trustedRootCertificates = trustedRootCertificates1}}));

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
        obj->ice_ping();
    }
    catch (const Ice::ConnectionLostException&)
    {
        // Expected
    }
    catch (...)
    {
        CFRelease(serverCertificateChain);
        CFRelease(clientCertificateChain);
        CFRelease(trustedRootCertificates1);
        CFRelease(trustedRootCertificates2);
        throw;
    }
    CFRelease(serverCertificateChain);
    CFRelease(clientCertificateChain);
    CFRelease(trustedRootCertificates1);
    CFRelease(trustedRootCertificates2);
    cout << "ok" << endl;
}

void
serverRejectsClientUsingDefaultTrustedRootCertificates(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "server rejects client using default root certificates... " << flush;
    CFArrayRef serverCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/s_rsa_ca1.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef clientCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/c_rsa_ca1.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef trustedRootCertificates = SecureTransport::loadCACertificates(certificatesPath + "/cacert1.pem");
    try
    {
        auto serverAuthenticationOptions = Ice::SSL::ServerAuthenticationOptions{
            .serverCertificateSelectionCallback =
                [serverCertificateChain](const string&)
            {
                CFRetain(serverCertificateChain);
                return serverCertificateChain;
            },
            .clientCertificateRequired = kAlwaysAuthenticate};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::CommunicatorHolder clientCommunicator(initialize(Ice::InitializationData{
            .clientAuthenticationOptions = Ice::SSL::ClientAuthenticationOptions{
                .clientCertificateSelectionCallback =
                    [clientCertificateChain](const string&)
                {
                    CFRetain(clientCertificateChain);
                    return clientCertificateChain;
                },
                .trustedRootCertificates = trustedRootCertificates}}));

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
        obj->ice_ping();
    }
    catch (const Ice::ConnectionLostException&)
    {
        // Expected
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
    cout << "server rejects client using validation callback... " << flush;
    CFArrayRef serverCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/s_rsa_ca1.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef clientCertificateChain = SecureTransport::loadCertificateChain(
        certificatesPath + "/c_rsa_ca1.p12",
        "",
        getKeyChainPath(certificatesPath),
        keychainPassword,
        password);
    CFArrayRef trustedRootCertificates = SecureTransport::loadCACertificates(certificatesPath + "/cacert1.pem");
    try
    {
        auto serverAuthenticationOptions = Ice::SSL::ServerAuthenticationOptions{
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

        Ice::CommunicatorHolder clientCommunicator(initialize(Ice::InitializationData{
            .clientAuthenticationOptions = Ice::SSL::ClientAuthenticationOptions{
                .clientCertificateSelectionCallback =
                    [clientCertificateChain](const string&)
                {
                    CFRetain(clientCertificateChain);
                    return clientCertificateChain;
                },
                .trustedRootCertificates = trustedRootCertificates}}));

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
        obj->ice_ping();
    }
    catch (const Ice::ConnectionLostException&)
    {
        // Expected
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

#    ifdef ICE_USE_SECURE_TRANSPORT_IOS
void
allSecureTransportTests(Test::TestHelper* helper, const string&)
{
    const string certificatesPath = getResourcePath("certs");
#    else
void
allSecureTransportTests(Test::TestHelper* helper, const string& testDir)
{
    const string certificatesPath = testDir + "/../certs";
#    endif
    clientValidatesServerSettingTrustedRootCertificates(helper, certificatesPath);
    clientRejectsServerSettingTrustedRootCertificates(helper, certificatesPath);
    clientRejectsServerUsingDefaultTrustedRootCertificates(helper, certificatesPath);
    clientRejectsServerUsingValidationCallback(helper, certificatesPath);

    serverValidatesClientSettingTrustedRootCertificates(helper, certificatesPath);
    serverRejectsClientSettingTrustedRootCertificates(helper, certificatesPath);
    serverRejectsClientUsingDefaultTrustedRootCertificates(helper, certificatesPath);
    serverRejectsClientUsingValidationCallback(helper, certificatesPath);
}
#endif
