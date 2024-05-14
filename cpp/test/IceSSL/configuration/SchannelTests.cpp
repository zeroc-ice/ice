//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "../../src/Ice/SSL/SecureTransportUtil.h"
#include "Ice/SSL/ClientAuthenticationOptions.h"
#include "Ice/SSL/ServerAuthenticationOptions.h"
#include "Test.h"
#include "TestHelper.h"
#include "TestI.h"

#include <algorithm>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

using namespace std;
using namespace Ice;
using namespace Ice::SSL;
using namespace Test;

#ifdef ICE_USE_SCHANNEL

const string password = "password";

vector<unsigned char>
readFile(const string& path)
{
    ifstream file(path, ios::binary);
    if (!file)
    {
        throw runtime_error("Unable to read certificate file");
    }
    return vector<unsigned char>((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
}

HCERTSTORE
loadPfxStore(const string& path)
{
    vector<unsigned char> buffer = readFile(path);
    if (buffer.empty())
    {
        throw runtime_error("The PFX file buffer is empty.");
    }

    CRYPT_DATA_BLOB pfxBlob;
    pfxBlob.cbData = static_cast<DWORD>(buffer.size());
    pfxBlob.pbData = reinterpret_cast<BYTE*>(&buffer[0]);

    HCERTSTORE store = PFXImportCertStore(&pfxBlob, L"password", CRYPT_USER_KEYSET);
    if (store == nullptr)
    {
        throw runtime_error("Unable to import certificate");
    }
    return store;
}

PCCERT_CONTEXT
loadCertificateContext(const string& certificatePath)
{
    PCCERT_CONTEXT cert = nullptr;
    HCERTSTORE store = loadPfxStore(certificatePath);
    CERT_CHAIN_FIND_BY_ISSUER_PARA para{0};
    para.cbSize = sizeof(CERT_CHAIN_FIND_BY_ISSUER_PARA);

    PCCERT_CHAIN_CONTEXT chain = nullptr;
    while (!cert)
    {
        chain = CertFindChainInStore(
            store,
            X509_ASN_ENCODING,
            CERT_CHAIN_FIND_BY_ISSUER_CACHE_ONLY_URL_FLAG, // Don't fetch anything from the Internet
            CERT_CHAIN_FIND_BY_ISSUER,
            &para,
            chain);
        if (!chain)
        {
            break; // No more chains found in the store.
        }

        if (chain->cChain > 0 && chain->rgpChain[0]->cElement > 0)
        {
            cert = CertDuplicateCertificateContext(chain->rgpChain[0]->rgpElement[0]->pCertContext);
        }
        CertFreeCertificateChain(chain);
    }

    // Check if we can find a certificate if we couldn't find a chain.
    if (!cert)
    {
        cert = CertFindCertificateInStore(store, X509_ASN_ENCODING, 0, CERT_FIND_ANY, 0, cert);
    }

    if (!cert)
    {
        throw runtime_error("Unable to import certificate");
    }
    return cert;
}

HCERTSTORE
loadTrustedRootCertificates(const string& path)
{
    HCERTSTORE store = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, 0, 0, nullptr);
    if (!store)
    {
        throw runtime_error("Unable to open memory store");
    }

    vector<unsigned char> buffer = readFile(path);
    if (buffer.empty())
    {
        CertCloseStore(store, 0);
        throw runtime_error("Buffer is empty");
    }

    if (!CertAddEncodedCertificateToStore(
            store,
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            &buffer[0],
            static_cast<DWORD>(buffer.size()),
            CERT_STORE_ADD_NEW,
            nullptr))
    {
        DWORD dwError = GetLastError();
        CertCloseStore(store, 0);
        throw runtime_error("Error adding certificate to store, Error Code: " + to_string(dwError));
    }
    return store;
}

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

Ice::CommunicatorPtr
createClient(optional<ClientAuthenticationOptions> clientAuthenticationOptions)
{
    Ice::InitializationData initData;
    initData.properties = Ice::createProperties();
    if (clientAuthenticationOptions)
    {
        initData.clientAuthenticationOptions = *clientAuthenticationOptions;
    }
    return initialize(initData);
}

void
clientValidatesServerSettingTrustedRootCertificates(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "client validates server certificate setting trusted root certificates... " << flush;
    PCCERT_CONTEXT serverCertificate = loadCertificateContext(certificatesPath + "/s_rsa_ca1.p12");
    HCERTSTORE trustedRootCertificates = loadTrustedRootCertificates(certificatesPath + "/cacert1.der");

    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverCertificateSelectionCallback = [serverCertificate](const string&)
            {
                CertDuplicateCertificateContext(serverCertificate);
                return serverCertificate;
            }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::SSL::ClientAuthenticationOptions ClientAuthenticationOptions{
            .trustedRootCertificates = trustedRootCertificates};
        Ice::CommunicatorHolder clientCommunicator(createClient(ClientAuthenticationOptions));

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
        obj->ice_ping();
    }
    catch (...)
    {
        CertFreeCertificateContext(serverCertificate);
        CertCloseStore(trustedRootCertificates, 0);
        throw;
    }
    CertFreeCertificateContext(serverCertificate);
    CertCloseStore(trustedRootCertificates, 0);
    cout << "ok" << endl;
}

void
clientRejectsServerSettingTrustedRootCertificates(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "client rejects server certificate setting trusted root certificates... " << flush;
    PCCERT_CONTEXT serverCertificate = loadCertificateContext(certificatesPath + "/s_rsa_ca1.p12");
    HCERTSTORE trustedRootCertificates = loadTrustedRootCertificates(certificatesPath + "/cacert2.der");
    try
    {
        auto serverAuthenticationOptions = Ice::SSL::ServerAuthenticationOptions{
            .serverCertificateSelectionCallback = [serverCertificate](const string&)
            {
                CertDuplicateCertificateContext(serverCertificate);
                return serverCertificate;
            }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::CommunicatorHolder clientCommunicator(initialize(Ice::InitializationData{
            .clientAuthenticationOptions =
                Ice::SSL::ClientAuthenticationOptions{.trustedRootCertificates = trustedRootCertificates}}));

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
        CertFreeCertificateContext(serverCertificate);
        CertCloseStore(trustedRootCertificates, 0);
        throw;
    }
    CertFreeCertificateContext(serverCertificate);
    CertCloseStore(trustedRootCertificates, 0);
    cout << "ok" << endl;
}

void
clientRejectsServerUsingDefaultTrustedRootCertificates(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "client rejects server certificate using default trusted root certificates... " << flush;
    PCCERT_CONTEXT serverCertificate = loadCertificateContext(certificatesPath + "/s_rsa_ca1.p12");
    try
    {
        auto serverAuthenticationOptions = Ice::SSL::ServerAuthenticationOptions{
            .serverCertificateSelectionCallback = [serverCertificate](const string&)
            {
                CertDuplicateCertificateContext(serverCertificate);
                return serverCertificate;
            }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::CommunicatorHolder clientCommunicator(initialize(
            Ice::InitializationData{.clientAuthenticationOptions = Ice::SSL::ClientAuthenticationOptions{}}));

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
        CertFreeCertificateContext(serverCertificate);
        throw;
    }
    CertFreeCertificateContext(serverCertificate);
    cout << "ok" << endl;
}

void
clientRejectsServerUsingValidationCallback(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "client rejects server certificate using validation callback... " << flush;
    PCCERT_CONTEXT serverCertificate = loadCertificateContext(certificatesPath + "/s_rsa_ca1.p12");
    HCERTSTORE trustedRootCertificates = loadTrustedRootCertificates(certificatesPath + "/cacert1.der");
    try
    {
        auto serverAuthenticationOptions = Ice::SSL::ServerAuthenticationOptions{
            .serverCertificateSelectionCallback = [serverCertificate](const string&)
            {
                CertDuplicateCertificateContext(serverCertificate);
                return serverCertificate;
            }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::CommunicatorHolder clientCommunicator(initialize(Ice::InitializationData{
            .clientAuthenticationOptions = Ice::SSL::ClientAuthenticationOptions{
                .trustedRootCertificates = trustedRootCertificates,
                .serverCertificateValidationCallback = [](CtxtHandle, const Ice::SSL::ConnectionInfoPtr&)
                { return false; }}}));

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
        CertFreeCertificateContext(serverCertificate);
        CertCloseStore(trustedRootCertificates, 0);
        throw;
    }
    CertFreeCertificateContext(serverCertificate);
    CertCloseStore(trustedRootCertificates, 0);
    cout << "ok" << endl;
}

void
serverValidatesClientSettingTrustedRootCertificates(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "server validates client certificate setting trusted root certificates... " << flush;
    PCCERT_CONTEXT serverCertificate = loadCertificateContext(certificatesPath + "/s_rsa_ca1.p12");
    PCCERT_CONTEXT clientCertificate = loadCertificateContext(certificatesPath + "/c_rsa_ca1.p12");
    HCERTSTORE trustedRootCertificates = loadTrustedRootCertificates(certificatesPath + "/cacert1.der");
    try
    {
        auto serverAuthenticationOptions = Ice::SSL::ServerAuthenticationOptions{
            .serverCertificateSelectionCallback =
                [serverCertificate](const string&)
            {
                CertDuplicateCertificateContext(serverCertificate);
                return serverCertificate;
            },
            .trustedRootCertificates = trustedRootCertificates};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::CommunicatorHolder clientCommunicator(initialize(Ice::InitializationData{
            .clientAuthenticationOptions = Ice::SSL::ClientAuthenticationOptions{
                .clientCertificateSelectionCallback =
                    [clientCertificate](const string&)
                {
                    CertDuplicateCertificateContext(clientCertificate);
                    return clientCertificate;
                },
                .trustedRootCertificates = trustedRootCertificates}}));

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(10, "ssl"));
        obj->ice_ping();
    }
    catch (...)
    {
        CertFreeCertificateContext(serverCertificate);
        CertCloseStore(trustedRootCertificates, 0);
        throw;
    }
    CertFreeCertificateContext(serverCertificate);
    CertCloseStore(trustedRootCertificates, 0);
    cout << "ok" << endl;
}

void
serverRejectsClientSettingTrustedRootCertificates(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "server rejects client certificate setting trusted root certificates... " << flush;
    PCCERT_CONTEXT serverCertificate = loadCertificateContext(certificatesPath + "/s_rsa_ca1.p12");
    PCCERT_CONTEXT clientCertificate = loadCertificateContext(certificatesPath + "/c_rsa_ca1.p12");
    HCERTSTORE trustedRootCertificates1 = loadTrustedRootCertificates(certificatesPath + "/cacert1.der");
    HCERTSTORE trustedRootCertificates2 = loadTrustedRootCertificates(certificatesPath + "/cacert2.der");
    try
    {
        auto serverAuthenticationOptions = Ice::SSL::ServerAuthenticationOptions{
            .serverCertificateSelectionCallback =
                [serverCertificate](const string&)
            {
                CertDuplicateCertificateContext(serverCertificate);
                return serverCertificate;
            },
            .clientCertificateRequired = true,
            // The server CA doesn't trust the client certificate.
            .trustedRootCertificates = trustedRootCertificates2};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::CommunicatorHolder clientCommunicator(initialize(Ice::InitializationData{
            .clientAuthenticationOptions = Ice::SSL::ClientAuthenticationOptions{
                .clientCertificateSelectionCallback =
                    [clientCertificate](const string&)
                {
                    CertDuplicateCertificateContext(clientCertificate);
                    return clientCertificate;
                },
                .trustedRootCertificates = trustedRootCertificates1}}));

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
        CertFreeCertificateContext(serverCertificate);
        CertCloseStore(trustedRootCertificates1, 0);
        CertCloseStore(trustedRootCertificates2, 0);
        throw;
    }
    CertFreeCertificateContext(serverCertificate);
    CertCloseStore(trustedRootCertificates1, 0);
    CertCloseStore(trustedRootCertificates2, 0);
    cout << "ok" << endl;
}

void
serverRejectsClientUsingDefaultTrustedRootCertificates(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "server rejects client certificate using default trusted root certificates... " << flush;
    PCCERT_CONTEXT serverCertificate = loadCertificateContext(certificatesPath + "/s_rsa_ca1.p12");
    PCCERT_CONTEXT clientCertificate = loadCertificateContext(certificatesPath + "/c_rsa_ca1.p12");
    HCERTSTORE trustedRootCertificates = loadTrustedRootCertificates(certificatesPath + "/cacert1.der");
    try
    {
        // No trusted root certificates are set on the server, it would use the system trusted root
        // certificates.
        auto serverAuthenticationOptions = Ice::SSL::ServerAuthenticationOptions{
            .serverCertificateSelectionCallback =
                [serverCertificate](const string&)
            {
                CertDuplicateCertificateContext(serverCertificate);
                return serverCertificate;
            },
            .clientCertificateRequired = true,
        };
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::CommunicatorHolder clientCommunicator(initialize(Ice::InitializationData{
            .clientAuthenticationOptions = Ice::SSL::ClientAuthenticationOptions{
                .clientCertificateSelectionCallback =
                    [clientCertificate](const string&)
                {
                    CertDuplicateCertificateContext(clientCertificate);
                    return clientCertificate;
                },
                .trustedRootCertificates = trustedRootCertificates}}));

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
        CertFreeCertificateContext(serverCertificate);
        CertCloseStore(trustedRootCertificates, 0);
        throw;
    }
    CertFreeCertificateContext(serverCertificate);
    CertCloseStore(trustedRootCertificates, 0);
    cout << "ok" << endl;
}

void
serverRejectsClientUsingValidationCallback(Test::TestHelper* helper, const string& certificatesPath)
{
    cout << "server rejects client certificate using validation callback... " << flush;
    PCCERT_CONTEXT serverCertificate = loadCertificateContext(certificatesPath + "/s_rsa_ca1.p12");
    PCCERT_CONTEXT clientCertificate = loadCertificateContext(certificatesPath + "/c_rsa_ca1.p12");
    HCERTSTORE trustedRootCertificates = loadTrustedRootCertificates(certificatesPath + "/cacert1.der");
    try
    {
        // The server configured trusted root certificates, trust the client certificate, but the validation
        // callback rejects the client certificate.
        auto serverAuthenticationOptions = Ice::SSL::ServerAuthenticationOptions{
            .serverCertificateSelectionCallback =
                [serverCertificate](const string&)
            {
                CertDuplicateCertificateContext(serverCertificate);
                return serverCertificate;
            },
            .clientCertificateRequired = true,
            .trustedRootCertificates = trustedRootCertificates,
            .clientCertificateValidationCallback = [](CtxtHandle, const Ice::SSL::ConnectionInfoPtr&)
            { return false; }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::CommunicatorHolder clientCommunicator(initialize(Ice::InitializationData{
            .clientAuthenticationOptions = Ice::SSL::ClientAuthenticationOptions{
                .clientCertificateSelectionCallback =
                    [clientCertificate](const string&)
                {
                    CertDuplicateCertificateContext(clientCertificate);
                    return clientCertificate;
                },
                .trustedRootCertificates = trustedRootCertificates}}));

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
        CertFreeCertificateContext(serverCertificate);
        CertCloseStore(trustedRootCertificates, 0);
        throw;
    }
    CertFreeCertificateContext(serverCertificate);
    CertCloseStore(trustedRootCertificates, 0);
    cout << "ok" << endl;
}

void
allSchannelTests(Test::TestHelper* helper, const string& testDir)
{
    const string certificatesPath = testDir + "/../certs";

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
