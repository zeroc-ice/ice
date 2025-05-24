// Copyright (c) ZeroC, Inc.

#include "../../src/Ice/SSL/SecureTransportUtil.h" // For loadCertificateChain
#include "../../src/Ice/UniqueRef.h"
#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;
using namespace Ice;

string
toHexString(vector<uint8_t> data)
{
    ostringstream os;
    for (auto i = data.begin(); i != data.end();)
    {
        unsigned char c = *i;
        os.fill('0');
        os.width(2);
        os << hex << uppercase << static_cast<int>(c);
        if (++i != data.end())
        {
            os << ':';
        }
    }
    return os.str();
}

void
readFile(const string& file, vector<char>& buffer)
{
    ifstream is(file.c_str(), ios::in | ios::binary);
    if (!is.good())
    {
        throw runtime_error("error opening file " + file);
    }

    is.seekg(0, is.end);
    buffer.resize(static_cast<size_t>(is.tellg()));
    is.seekg(0, is.beg);

    is.read(&buffer[0], static_cast<streamsize>(buffer.size()));

    if (!is.good())
    {
        throw runtime_error("error reading file " + file);
    }
}

#ifdef ICE_USE_SCHANNEL
class ImportCerts
{
public:
    ImportCerts(const string& defaultDir, const char* certificates[])
    {
        //
        // First we need to import some certificates in the user store.
        //
        _store = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, 0, CERT_SYSTEM_STORE_CURRENT_USER, L"MY");
        test(_store);

        for (int i = 0; certificates[i] != 0; ++i)
        {
            vector<char> buffer;
            readFile(defaultDir + certificates[i], buffer);

            CRYPT_DATA_BLOB p12Blob;
            p12Blob.cbData = static_cast<DWORD>(buffer.size());
            p12Blob.pbData = reinterpret_cast<BYTE*>(&buffer[0]);

            HCERTSTORE p12 = PFXImportCertStore(&p12Blob, L"password", CRYPT_USER_KEYSET);
            _stores.push_back(p12);

            PCCERT_CONTEXT next = 0;
            PCCERT_CONTEXT newCert = 0;
            do
            {
                if ((next = CertFindCertificateInStore(
                         p12,
                         X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                         0,
                         CERT_FIND_ANY,
                         0,
                         next)) != 0)
                {
                    if (CertAddCertificateContextToStore(_store, next, CERT_STORE_ADD_ALWAYS, &newCert))
                    {
                        _certs.push_back(newCert);
                    }
                }
            } while (next);
        }
    }

    ~ImportCerts() { cleanup(); }

    void cleanup()
    {
        for (vector<PCCERT_CONTEXT>::const_iterator i = _certs.begin(); i != _certs.end(); ++i)
        {
            PCCERT_CONTEXT cert = *i;

            // Retrieve the certificate CERT_KEY_PROV_INFO_PROP_ID property, we use the CRYPT_KEY_PROV_INFO
            // data to then remove the key set associated with the certificate.
            DWORD size = 0;
            if (CertGetCertificateContextProperty(cert, CERT_KEY_PROV_INFO_PROP_ID, 0, &size))
            {
                vector<char> buf(size);
                if (CertGetCertificateContextProperty(cert, CERT_KEY_PROV_INFO_PROP_ID, &buf[0], &size))
                {
                    CRYPT_KEY_PROV_INFO* keyProvInfo = reinterpret_cast<CRYPT_KEY_PROV_INFO*>(&buf[0]);
                    HCRYPTPROV cryptProv = 0;
                    if (CryptAcquireContextW(
                            &cryptProv,
                            keyProvInfo->pwszContainerName,
                            keyProvInfo->pwszProvName,
                            keyProvInfo->dwProvType,
                            0))
                    {
                        CryptAcquireContextW(
                            &cryptProv,
                            keyProvInfo->pwszContainerName,
                            keyProvInfo->pwszProvName,
                            keyProvInfo->dwProvType,
                            CRYPT_DELETEKEYSET);
                    }
                }
            }
            CertDeleteCertificateFromStore(cert);
        }
        _certs.clear();
        for (vector<HCERTSTORE>::const_iterator i = _stores.begin(); i != _stores.end(); ++i)
        {
            CertCloseStore(*i, 0);
        }
        _stores.clear();
        if (_store)
        {
            CertCloseStore(_store, 0);
            _store = 0;
        }
    }

private:
    HCERTSTORE _store;
    vector<HCERTSTORE> _stores;
    vector<PCCERT_CONTEXT> _certs;
};

#elif defined(__APPLE__) && TARGET_OS_IPHONE != 0
class ImportCerts
{
public:
    ImportCerts(const string& defaultDir, const char* certificates[])
    {
        for (int i = 0; certificates[i] != 0; ++i)
        {
            optional<string> resolved = Ice::SSL::resolveFilePath(certificates[i], defaultDir);
            if (resolved)
            {
                IceInternal::UniqueRef<CFArrayRef> certs(
                    Ice::SSL::SecureTransport::loadCertificateChain(*resolved, "", "", "", "password"));
                SecIdentityRef identity =
                    static_cast<SecIdentityRef>(const_cast<void*>(CFArrayGetValueAtIndex(certs.get(), 0)));
                CFRetain(identity);
                _identities.push_back(identity);
                OSStatus err;
                IceInternal::UniqueRef<CFMutableDictionaryRef> query;

                query.reset(
                    CFDictionaryCreateMutable(0, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
                CFDictionarySetValue(query.get(), kSecValueRef, identity);
                if ((err = SecItemAdd(query.get(), 0)))
                {
                    cerr << "failed to add identity " << certificates[i] << ": " << err << endl;
                }
            }
        }
    }

    ~ImportCerts() { cleanup(); }

    void cleanup()
    {
        IceInternal::UniqueRef<CFMutableDictionaryRef> query;
        for (vector<SecIdentityRef>::const_iterator p = _identities.begin(); p != _identities.end(); ++p)
        {
            query.reset(
                CFDictionaryCreateMutable(0, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
            CFDictionarySetValue(query.get(), kSecClass, kSecClassIdentity);
            CFDictionarySetValue(query.get(), kSecValueRef, *p);
            SecItemDelete(query.get());

            SecCertificateRef cert;
            SecIdentityCopyCertificate(*p, &cert);
            query.reset(
                CFDictionaryCreateMutable(0, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
            CFDictionarySetValue(query.get(), kSecClass, kSecClassCertificate);
            CFDictionarySetValue(query.get(), kSecValueRef, cert);
            SecItemDelete(query.get());
            CFRelease(*p);
        }
        _identities.clear();
    }

private:
    vector<SecIdentityRef> _identities;
};
#else
class ImportCerts
{
public:
    ImportCerts(const string& /*defaultDir*/, const char* /*certificates*/[])
    {
        // Nothing to do.
    }

    void cleanup() {}
};
#endif

int keychainN = 0;

static PropertiesPtr
createClientProps(const Ice::PropertiesPtr& defaultProps, bool p12)
{
    string defaultDir = defaultProps->getIceProperty("IceSSL.DefaultDir");
    PropertiesPtr result = createProperties();
    result->setProperty("IceSSL.DefaultDir", defaultDir);
    result->setProperty("Ice.Default.Host", defaultProps->getIceProperty("Ice.Default.Host"));
    if (!defaultProps->getIceProperty("Ice.IPv6").empty())
    {
        result->setProperty("Ice.IPv6", defaultProps->getIceProperty("Ice.IPv6"));
    }
    if (p12)
    {
        result->setProperty("IceSSL.Password", "password");
    }
#ifdef ICE_USE_SECURE_TRANSPORT
    ostringstream keychainName;
    keychainName << defaultDir << "/keychain/client" << keychainN++ << ".keychain";
    const string keychainPassword = "password";
    result->setProperty("IceSSL.Keychain", keychainName.str());
    result->setProperty("IceSSL.KeychainPassword", keychainPassword);
#endif
    return result;
}

static Test::Properties
createServerProps(const Ice::PropertiesPtr& defaultProps, bool p12)
{
    Test::Properties result;
    string defaultDir = defaultProps->getIceProperty("IceSSL.DefaultDir");
    result["IceSSL.DefaultDir"] = defaultDir;
    result["Ice.Default.Host"] = defaultProps->getIceProperty("Ice.Default.Host");
    result["ServerAdapter.PublishedHost"] = result["Ice.Default.Host"];

    if (!defaultProps->getIceProperty("Ice.IPv6").empty())
    {
        result["Ice.IPv6"] = defaultProps->getIceProperty("Ice.IPv6");
    }
    if (p12)
    {
        result["IceSSL.Password"] = "password";
    }
#ifdef ICE_USE_SECURE_TRANSPORT
    ostringstream keychainName;
    keychainName << defaultDir << "/keychain/server" << keychainN << ".keychain";
    result["IceSSL.Keychain"] = keychainName.str();
    result["IceSSL.KeychainPassword"] = "password";
#endif
    return result;
}

static Test::Properties
createServerProps(const Ice::PropertiesPtr& defaultProps, bool p12, const string& cert, const string& ca)
{
    Test::Properties d = createServerProps(defaultProps, p12);
    if (!ca.empty())
    {
        d["IceSSL.CAs"] = ca + "_cert.pem";
    }

    if (!cert.empty())
    {
        if (p12)
        {
            d["IceSSL.CertFile"] = cert + ".p12";
        }
        else
        {
            d["IceSSL.CertFile"] = cert + "_cert.pem";
            d["IceSSL.KeyFile"] = cert + "_key.pem";
        }
    }
    return d;
}

static PropertiesPtr
createClientProps(const Ice::PropertiesPtr& defaultProps, bool p12, const string& cert, const string& ca)
{
    Ice::PropertiesPtr properties = createClientProps(defaultProps, p12);
    if (!ca.empty())
    {
        properties->setProperty("IceSSL.CAs", ca + "_cert.pem");
    }

    if (!cert.empty())
    {
        if (p12)
        {
            properties->setProperty("IceSSL.CertFile", cert + ".p12");
        }
        else
        {
            properties->setProperty("IceSSL.CertFile", cert + "_cert.pem");
            properties->setProperty("IceSSL.KeyFile", cert + "_key.pem");
        }
    }
    return properties;
}

void
testCertificateVerification(
    const string& factoryRef,
    const string& defaultHost,
    const Ice::PropertiesPtr& defaultProps,
    bool p12)
{
    cout << "testing certificate verification... " << flush;
    Test::Properties d;
    optional<Test::ServerPrx> server;
    optional<Test::ServerFactoryPrx> fact;
    CommunicatorPtr comm;
    InitializationData initData;

    // Test IceSSL.VerifyPeer=0. Client does not have a certificate, and doesn't trust the server certificate.
    initData.properties = createClientProps(defaultProps, p12, "", "");
    initData.properties->setProperty("IceSSL.VerifyPeer", "0");
    comm = initialize(initData);
    fact = Test::ServerFactoryPrx{comm, factoryRef};

    d = createServerProps(defaultProps, p12, "ca1/server", "");
    d["IceSSL.VerifyPeer"] = "0";
    server = fact->createServer(d);
    try
    {
        server->noCert();
        test(false);
    }
    catch (const Ice::SecurityException&)
    {
        // Expected.
    }
    catch (const ConnectionLostException&)
    {
        // Expected.
    }
    catch (const LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    fact->destroyServer(server);
    comm->destroy();

    // Test IceSSL.VerifyPeer=0. Client does not have a certificate, but it still verifies the server's.
    initData.properties = createClientProps(defaultProps, p12, "", "ca1/ca1");
    comm = initialize(initData);
    fact = Test::ServerFactoryPrx{comm, factoryRef};

    d = createServerProps(defaultProps, p12, "ca1/server", "");
    d["IceSSL.VerifyPeer"] = "0";
    server = fact->createServer(d);
    try
    {
        server->noCert();
        server->ice_getConnection()->close().get();
        server->noCert();
    }
    catch (const LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    fact->destroyServer(server);

    // Test IceSSL.VerifyPeer=1. Client does not have a certificate.
    d = createServerProps(defaultProps, p12, "ca1/server", "");
    d["IceSSL.VerifyPeer"] = "1";
    server = fact->createServer(d);
    try
    {
        server->noCert();
    }
    catch (const LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    fact->destroyServer(server);

    // Test IceSSL.VerifyPeer=2. This should fail because the client does not supply a certificate.
    d = createServerProps(defaultProps, p12, "ca1/server", "");
    d["IceSSL.VerifyPeer"] = "2";
    server = fact->createServer(d);
    try
    {
        server->ice_ping();
        test(false);
    }
    catch (const ProtocolException&)
    {
        // Expected, if reported as an SSL alert by the server.
    }
    catch (const ConnectionLostException&)
    {
        // Expected.
    }
    catch (const LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    fact->destroyServer(server);
    comm->destroy();

    // Test IceSSL.VerifyPeer=1. This should fail because the client doesn't trust the server's CA.
    initData.properties = createClientProps(defaultProps, p12, "", "");
    initData.properties->setProperty("IceSSL.VerifyPeer", "1");
    comm = initialize(initData);
    fact = Test::ServerFactoryPrx{comm, factoryRef};
    d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
    d["IceSSL.VerifyPeer"] = "0";
    server = fact->createServer(d);
    try
    {
        server->ice_ping();
        test(false);
    }
    catch (const SecurityException&)
    {
        // Expected.
    }
    catch (const LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    fact->destroyServer(server);

    comm->destroy();

    // Test IceSSL.VerifyPeer=1. This should fail because the server doesn't trust the client's CA.
    initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
    initData.properties->setProperty("IceSSL.VerifyPeer", "0");
    comm = initialize(initData);
    fact = Test::ServerFactoryPrx{comm, factoryRef};
    d = createServerProps(defaultProps, p12, "ca1/server", "");
    d["IceSSL.VerifyPeer"] = "1";
    server = fact->createServer(d);
    try
    {
        server->ice_ping();
        test(false);
    }
    catch (const ConnectionLostException&)
    {
        // Expected.
    }
    catch (const LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    fact->destroyServer(server);
    comm->destroy();

    // This should succeed because the self signed certificate used by the server is trusted.
    initData.properties = createClientProps(defaultProps, p12, "", "ca2/ca2");
    comm = initialize(initData);
    fact = Test::ServerFactoryPrx{comm, factoryRef};
    d = createServerProps(defaultProps, p12, "ca2/ca2", "");
    d["IceSSL.VerifyPeer"] = "0";
    server = fact->createServer(d);
    try
    {
        server->ice_ping();
    }
    catch (const LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    fact->destroyServer(server);
    comm->destroy();

    // This should fail because the self signed certificate used by the server is not trusted.
    initData.properties = createClientProps(defaultProps, p12, "", "");
    comm = initialize(initData);
    fact = Test::ServerFactoryPrx{comm, factoryRef};
    d = createServerProps(defaultProps, p12, "ca2/ca2", "");
    d["IceSSL.VerifyPeer"] = "0";
    server = fact->createServer(d);
    try
    {
        server->ice_ping();
        test(false);
    }
    catch (const SecurityException&)
    {
        // Expected.
    }
    catch (const LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    fact->destroyServer(server);
    comm->destroy();

    // Verify that IceSSL.CheckCertName has no effect in a server.
    initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
    initData.properties->setProperty("IceSSL.CheckCertName", "0");
    comm = initialize(initData);
    fact = Test::ServerFactoryPrx{comm, factoryRef};
    d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
    d["IceSSL.VerifyPeer"] = "2";
    d["IceSSL.CheckCertName"] = "1";
    server = fact->createServer(d);
    try
    {
        server->ice_ping();
    }
    catch (const LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    fact->destroyServer(server);
    comm->destroy();

    // Test Hostname verification only when Ice.DefaultHost is 127.0.0.1 as that is the IP address used in the test
    // certificates.
    if (defaultHost == "127.0.0.1")
    {
        // Test using localhost as target host
        Ice::PropertiesPtr props = defaultProps->clone();
        props->setProperty("Ice.Default.Host", "localhost");

        // Target host matches the certificate DNS altName
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty("IceSSL.CheckCertName", "1");
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");
        comm = initialize(initData);

        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        d = createServerProps(props, p12, "ca1/server_cn1", "ca1/ca1");
        server = fact->createServer(d);

        server->ice_ping();

        fact->destroyServer(server);
        comm->destroy();

        // Target host does not match the certificate DNS altName
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty("IceSSL.CheckCertName", "1");
        comm = initialize(initData);

        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        d = createServerProps(props, p12, "ca1/server_cn2", "ca1/ca1");
        server = fact->createServer(d);

        try
        {
            server->ice_ping();
        }
        catch (const Ice::SecurityException&)
        {
            // Expected.
        }

        fact->destroyServer(server);
        comm->destroy();

        // Target host matches the certificate Common Name and the certificate does not include a DNS altName
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty("IceSSL.CheckCertName", "1");
        comm = initialize(initData);

        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        d = createServerProps(props, p12, "ca1/server_cn3", "ca1/ca1");
        server = fact->createServer(d);

#if defined(__APPLE__)
        try
        {
            server->ice_ping();
        }
        catch (const Ice::SecurityException&)
        {
            // Expected.
        }
#else
        server->ice_ping();
#endif

        fact->destroyServer(server);
        comm->destroy();

        // Target host does not match the certificate Common Name and the certificate does not include a DNS altName
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty("IceSSL.CheckCertName", "1");
        comm = initialize(initData);

        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        d = createServerProps(props, p12, "ca1/server_cn4", "ca1/ca1");
        server = fact->createServer(d);

        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const Ice::SecurityException&)
        {
        }

        fact->destroyServer(server);
        comm->destroy();

        // Target host matches the certificate Common Name and the certificate has a DNS altName that does not
        // matches the target host.
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty("IceSSL.CheckCertName", "1");
        comm = initialize(initData);

        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        d = createServerProps(props, p12, "ca1/server_cn5", "ca1/ca1");
        server = fact->createServer(d);

        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const Ice::SecurityException&)
        {
        }

        fact->destroyServer(server);
        comm->destroy();

        // Test using 127.0.0.1 as target host

        // Target host matches the certificate IP altName
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty("IceSSL.CheckCertName", "1");
        comm = initialize(initData);

        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        d = createServerProps(defaultProps, p12, "ca1/server_cn6", "ca1/ca1");
        server = fact->createServer(d);

        server->ice_ping();

        fact->destroyServer(server);
        comm->destroy();

        // Target host does not match the certificate IP altName
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty("IceSSL.CheckCertName", "1");
        comm = initialize(initData);

        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        d = createServerProps(defaultProps, p12, "ca1/server_cn7", "ca1/ca1");
        server = fact->createServer(d);

        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const Ice::SecurityException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();

        // Target host is an IP address that matches the CN and the certificate doesn't include an IP altName.
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty("IceSSL.CheckCertName", "1");
        comm = initialize(initData);

        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        d = createServerProps(defaultProps, p12, "ca1/server_cn8", "ca1/ca1");
        server = fact->createServer(d);

        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const Ice::SecurityException&)
        {
        }

        fact->destroyServer(server);
        comm->destroy();
    }
    cout << "ok" << endl;
}

#if defined(ICE_USE_SCHANNEL)
void
testFindCert(const string& factoryRef, const string& defaultDir, const Ice::PropertiesPtr& defaultProps, bool p12)
{
    cout << "testing IceSSL.FindCert... " << flush;
    const char* clientFindCertProperties[] = {
        "SUBJECTDN:'CN=ca1.client, C=US, S=Florida, L=Jupiter, O=ZeroC, OU=Ice test infrastructure, E=info@zeroc.com'",
        "ISSUER:ca1 SUBJECT:ca1.client SERIAL:'10 01'",
        "ISSUERDN:'C=US, S=Florida, L=Jupiter, O=ZeroC, OU=Ice test infrastructure, CN=ca1, E=info@zeroc.com' "
        "SUBJECT:ca1.client",
        "THUMBPRINT:'7E 28 35 0E 67 B7 39 C4 28 AF CC B8 8E EF AA B5 55 33 43 25'",
        "SUBJECTKEYID:'35 F5 EB 10 49 80 8D 0B 3D 25 67 AB 43 1A EC EB 3D CE 95 28'",
        0};

    const char* serverFindCertProperties[] = {
        "SUBJECTDN:'CN=ca1.server, C=US, S=Florida, L=Jupiter, O=ZeroC, OU=Ice test infrastructure, E=info@zeroc.com'",
        "ISSUER:ca1 SUBJECT:ca1.server SERIAL:'10 0A'",
        "ISSUERDN:'C=US, S=Florida, L=Jupiter, O=ZeroC, OU=Ice test infrastructure, CN=ca1, E=info@zeroc.com' "
        "SUBJECT:ca1.server",
        "THUMBPRINT:'B0 5B 31 83 9D AC A6 D1 3D B3 BF EC 6F E5 CB B8 27 F4 C5 E5'",
        "SUBJECTKEYID:'21 27 14 AD E7 AC 9A 3D A7 27 24 EA 33 E7 07 2F 97 17 B1 AF'",
        0};

    const char* failFindCertProperties[] = {
        "nolabel",
        "unknownlabel:foo",
        "LABEL:",
        "SUBJECTDN:'CN = Client, E = infox@zeroc.com, OU = Ice, O = \"ZeroC, Inc.\", S = Florida, C = US'",
        "ISSUER:'ZeroC, Inc.' SUBJECT:Client SERIAL:'02 02'",
        "ISSUERDN:'E = info@zeroc.com, CN = \"ZeroC Test CA 1\", OU = Ice, O = \"ZeroC, Inc.\","
        " L = \"Palm Beach Gardens\", S = Florida, C = ES' SUBJECT:Client",
        "THUMBPRINT:'27 e0 18 c9 23 12 6c f0 5c da fa 36 5a 4c 63 5a e2 53 07 ff'",
        "SUBJECTKEYID:'a6 42 aa 17 04 41 86 56 67 e4 04 64 59 34 30 c7 4c 6b ef ff'",
        0};

    const char* certificates[] = {"/ca1/server.p12", "/ca1/client.p12", 0};
    ImportCerts import{defaultDir, certificates};

    for (int i = 0; clientFindCertProperties[i] != 0; i++)
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12);
        initData.properties->setProperty("IceSSL.CAs", "ca1/ca1_cert.pem");
        initData.properties->setProperty("IceSSL.CertStore", "My");
        initData.properties->setProperty("IceSSL.CertStoreLocation", "CurrentUser");
        initData.properties->setProperty("IceSSL.FindCert", clientFindCertProperties[i]);

        // Use TrustOnly to ensure the peer has pick the expected certificate.
        initData.properties->setProperty("IceSSL.TrustOnly", "CN=ca1.server");

        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        d["IceSSL.CAs"] = "ca1/ca1_cert.pem";
        d["IceSSL.FindCert"] = serverFindCertProperties[i];

        // Use TrustOnly to ensure the peer has pick the expected certificate.
        d["IceSSL.TrustOnly"] = "CN=ca1.client";

        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            import.cleanup();
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }

    // These must fail because the search criteria does not match any certificates.
    for (int i = 0; failFindCertProperties[i] != 0; i++)
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12);
        initData.properties->setProperty("IceSSL.CAs", "ca1/ca1.pem");
        initData.properties->setProperty("IceSSL.FindCert", failFindCertProperties[i]);
        try
        {
            CommunicatorPtr comm = initialize(initData);
            cerr << failFindCertProperties[i] << endl;
            import.cleanup();
            test(false);
        }
        catch (const InitializationException&)
        {
            // expected
        }
        catch (const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            import.cleanup();
            test(false);
        }
    }

    import.cleanup();

    //
    // These must fail because we have already remove the certificates.
    //
    for (int i = 0; clientFindCertProperties[i] != 0; i++)
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12);
        initData.properties->setProperty("IceSSL.CAs", "ca1/ca1.pem");
        initData.properties->setProperty("IceSSL.FindCert", clientFindCertProperties[i]);
        try
        {
            CommunicatorPtr comm = initialize(initData);
            test(false);
        }
        catch (const InitializationException&)
        {
            // expected
        }
        catch (const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
    }
    cout << "ok" << endl;
}
#elif defined(ICE_USE_SECURE_TRANSPORT)
void
testFindCert(const string& factoryRef, const string& defaultDir, const Ice::PropertiesPtr& defaultProps, bool p12)
{
    cout << "testing IceSSL.FindCert... " << flush;
    const char* clientFindCertProperties[] = {
        "LABEL:ca1.client",
        "SUBJECTKEYID:'35 F5 EB 10 49 80 8D 0B 3D 25 67 AB 43 1A EC EB 3D CE 95 28'",
        "SERIAL:'10 01'",
        "SERIAL:'10 01' LABEL:ca1.client",
        nullptr};

    const char* serverFindCertProperties[] = {
        "LABEL:ca1.server",
        "SUBJECTKEYID:'21 27 14 AD E7 AC 9A 3D A7 27 24 EA 33 E7 07 2F 97 17 B1 AF'",
        "SERIAL:'10 0A'",
        "SERIAL:'10 0A' LABEL:ca1.server",
        nullptr};

    const char* failFindCertProperties[] = {
        "nolabel",
        "unknownlabel:foo",
        "LABEL:",
        "LABEL:'ServerX'",
        "SUBJECTKEYID:'a6 42 aa 17 04 41 86 56 67 e4 04 64 59 34 30 c7 4c 6b ef ff'",
        "SERIAL:04",
        "SERIAL:04 LABEL:ca1.client",
        nullptr};

    const char* certificates[] = {"/ca1/server.p12", "/ca1/client.p12", nullptr};
    ImportCerts import{defaultDir, certificates};

    for (int i = 0; clientFindCertProperties[i] != nullptr; i++)
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12);
        initData.properties->setProperty("IceSSL.CAs", "ca1/ca1_cert.pem");
        initData.properties->setProperty("IceSSL.Keychain", defaultDir + "/Find.keychain");
        initData.properties->setProperty("IceSSL.KeychainPassword", "password");
        initData.properties->setProperty("IceSSL.FindCert", clientFindCertProperties[i]);

// Use TrustOnly to ensure the peer has pick the expected certificate.
#    ifndef ICE_USE_SECURE_TRANSPORT_IOS
        initData.properties->setProperty("IceSSL.TrustOnly", "CN=ca1.server");
#    endif
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12);
        d["IceSSL.CAs"] = "ca1/ca1_cert.pem";
        d["IceSSL.Keychain"] = defaultDir + "/Find.keychain";
        d["IceSSL.KeychainPassword"] = "password";
        d["IceSSL.FindCert"] = serverFindCertProperties[i];

        // Use TrustOnly to ensure the peer has pick the expected certificate.
#    ifndef ICE_USE_SECURE_TRANSPORT_IOS
        d["IceSSL.TrustOnly"] = "CN=ca1.client";
#    endif
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }

    for (int i = 0; failFindCertProperties[i] != nullptr; i++)
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12);
        initData.properties->setProperty("IceSSL.Keychain", defaultDir + "/Find.keychain");
        initData.properties->setProperty("IceSSL.KeychainPassword", "password");
        initData.properties->setProperty("IceSSL.FindCert", failFindCertProperties[i]);
        try
        {
            CommunicatorPtr comm = initialize(initData);
            printf("failed %s", failFindCertProperties[i]);
            test(false);
        }
        catch (const InitializationException&)
        {
            // expected
        }
        catch (const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
    }
    cout << "ok" << endl;
}
#else
void
testFindCert(const string&, const string&, const Ice::PropertiesPtr&, bool)
{
    // Not supported with OpenSSL.
}
#endif

void
testExpiredCert(const string& factoryRef, const Ice::PropertiesPtr& defaultProps, bool p12)
{
    cout << "testing expired certificates... " << flush;
    {
        // This should fail because the server's certificate is expired.
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca5/ca5");
        CommunicatorPtr comm = initialize(initData);
        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca5/server_expired", "ca1/ca1");
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const SecurityException&)
        {
            // Expected.
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();

        // This should fail because the intermediary certificate is expired.
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca5/ca5");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx{comm, factoryRef};
        d = createServerProps(defaultProps, p12, "ca5/i1_expired/server", "ca1/ca1");
        server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const SecurityException&)
        {
            // Expected.
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();

        // This should fail because the client's certificate is expired.
        initData.properties = createClientProps(defaultProps, p12, "ca5/client_expired", "ca1/ca1");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx{comm, factoryRef};
        d = createServerProps(defaultProps, p12, "ca1/server", "ca5/ca5");
        server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const SecurityException&)
        {
            // Expected.
        }
        catch (const ConnectionLostException&)
        {
            // Expected.
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    cout << "ok" << endl;
}

void
testNotYerValidCert(const string& factoryRef, const Ice::PropertiesPtr& defaultProps, bool p12)
{
    cout << "testing not yet valid certificates... " << flush;
    {
        // This should fail because the server's certificate is not yet valid.
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca5/ca5");
        CommunicatorPtr comm = initialize(initData);
        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca5/server_notyet", "ca1/ca1");
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const SecurityException&)
        {
            // Expected.
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();

        // This should fail because the intermediary certificate is not yet valid.
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca5/ca5");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx{comm, factoryRef};
        d = createServerProps(defaultProps, p12, "ca5/i1_notyet/server", "ca1/ca1");
        server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const SecurityException&)
        {
            // Expected.
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();

        // This should fail because the client's certificate is not yet valid.
        initData.properties = createClientProps(defaultProps, p12, "ca5/client_notyet", "ca1/ca1");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx{comm, factoryRef};
        d = createServerProps(defaultProps, p12, "ca1/server", "ca5/ca5");
        server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const SecurityException&)
        {
            // Expected.
        }
        catch (const ConnectionLostException&)
        {
            // Expected.
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    cout << "ok" << endl;
}

void
testCertificateChains(
    const string& factoryRef,
    const string& defaultDir,
    const Ice::PropertiesPtr& defaultProps,
    bool p12)
{
    cout << "testing certificate chains... " << flush;

    // Schannel doesn't send the chain unless it is imported.
    const char* certificates[] = {"/ca1/i1/i2/server.p12", nullptr};
    ImportCerts import{defaultDir, certificates};

    InitializationData initData;
    initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
    CommunicatorPtr comm = initialize(initData);

    Test::ServerFactoryPrx fact{comm, factoryRef};

    Test::Properties d = createServerProps(defaultProps, p12, "ca1/i1/i2/server", "ca1/ca1");
    optional<Test::ServerPrx> server = fact->createServer(d);
    try
    {
        server->ice_ping();
    }
    catch (const Ice::LocalException& ex)
    {
        import.cleanup();
        cerr << ex << endl;
        test(false);
    }
    fact->destroyServer(server);
    comm->destroy();
    cout << "ok" << endl;
}

#ifdef ICE_USE_OPENSSL
void
testCAsDirectory(const string& factoryRef, const string& defaultDir, const Ice::PropertiesPtr& defaultProps, bool p12)
{
    cout << "testing CA certificate directory... " << flush;
    // Don't specify CAs explicitly; we let OpenSSL find the CA certificate in the default directory.
    InitializationData initData;
    initData.properties = createClientProps(defaultProps, p12, "ca1/client", "");
    initData.properties->setProperty("IceSSL.CAs", defaultDir);
    CommunicatorPtr comm = initialize(initData);
    Test::ServerFactoryPrx fact{comm, factoryRef};
    Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "");
    d["IceSSL.CAs"] = defaultDir;
    optional<Test::ServerPrx> server = fact->createServer(d);
    try
    {
        server->ice_ping();
    }
    catch (const LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    fact->destroyServer(server);
    comm->destroy();
    cout << "ok" << endl;
}
#else
void
testCAsDirectory(const string&, const string&, const Ice::PropertiesPtr&, bool)
{
    // Not supported with Schannel and SecureTransport.
}
#endif

void
testMutlipleCACerts(const string& factoryRef, const Ice::PropertiesPtr& defaultProps, bool p12)
{
    cout << "testing multiple CA certificates... " << flush;
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca_all");
        CommunicatorPtr comm = initialize(initData);
        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca2/server", "ca_all");
        d["IceSSL.VerifyPeer"] = "2";
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch (const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    cout << "ok" << endl;
}

#ifndef ICE_USE_OPENSSL
void
testDerCertificates(const string& factoryRef, const Ice::PropertiesPtr& defaultProps, bool p12)
{
    // OpenSSL must use PEM certificate
    cout << "testing DER CA certificate... " << flush;
    InitializationData initData;
    initData.properties = createClientProps(defaultProps, p12, "ca1/client", "");
    initData.properties->setProperty("IceSSL.CAs", "ca1/ca1_cert.der");
    CommunicatorPtr comm = initialize(initData);
    Test::ServerFactoryPrx fact{comm, factoryRef};
    Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "");
    d["IceSSL.VerifyPeer"] = "2";
    d["IceSSL.CAs"] = "ca1/ca1_cert.der";
    optional<Test::ServerPrx> server = fact->createServer(d);
    try
    {
        server->ice_ping();
    }
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    fact->destroyServer(server);
    comm->destroy();
    cout << "ok" << endl;
}
#else
void
testDerCertificates(const string&, const Ice::PropertiesPtr&, bool)
{
    // Not supported with OpenSSL.
}
#endif

#ifdef ICE_USE_SECURE_TRANSPORT_IOS
void
testTrustOnly(const string&, const Ice::PropertiesPtr&, bool)
{
    // Test not supported on iOS
}
#else
void
testTrustOnly(const string& factoryRef, const Ice::PropertiesPtr& defaultProps, bool p12)
{
    cout << "testing IceSSL.TrustOnly... " << flush;

    // iOS support only provides access to the CN of the certificate so we can't check for other attributes
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty(
            "IceSSL.TrustOnly",
            "C=US, ST=Florida, O=ZeroC,"
            "OU=Ice test infrastructure, emailAddress=info@zeroc.com, CN=ca1.server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }

    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty(
            "IceSSL.TrustOnly",
            "!C=US, ST=Florida, O=ZeroC,"
            "OU=Ice test infrastructure, emailAddress=info@zeroc.com, CN=ca1.server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty(
            "IceSSL.TrustOnly",
            "C=US, ST=Florida, O=ZeroC,"
            "OU=Ice test infrastructure, emailAddress=info@zeroc.com, CN=ca1.server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        d["IceSSL.TrustOnly"] =
            "C=US, ST=Florida, O=ZeroC, OU=Ice test infrastructure, emailAddress=info@zeroc.com,CN=ca1.client";

        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        d["IceSSL.TrustOnly"] =
            "!C=US, ST=Florida, O=ZeroC, OU=Ice test infrastructure, emailAddress=info@zeroc.com, CN=ca1.client";

        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty("IceSSL.TrustOnly", "CN=ca1.server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty("IceSSL.TrustOnly", "!CN=ca1.server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        d["IceSSL.TrustOnly"] = "CN=ca1.client";
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        d["IceSSL.TrustOnly"] = "!CN=ca1.client";
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty("IceSSL.TrustOnly", "CN=ca1.client");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        d["IceSSL.TrustOnly"] = "CN=ca1.server";
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }

    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty("IceSSL.TrustOnly", "C=Canada,CN=ca1.server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty("IceSSL.TrustOnly", "!C=Canada,CN=ca1.server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty("IceSSL.TrustOnly", "C=Canada;CN=ca1.server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty("IceSSL.TrustOnly", "!C=Canada;!CN=ca1.server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty("IceSSL.TrustOnly", "!CN=ca1.server_x"); // Should not match "ca1.server"
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        d["IceSSL.TrustOnly"] = "!CN=ca1.client_x"; // Should not match "ca1.client"
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }

    {
        //
        // Test rejection when client does not supply a certificate.
        //
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "", "");
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        d["IceSSL.VerifyPeer"] = "0";
        d["IceSSL.TrustOnly"] = "CN=ca1.client";
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        //
        // Test rejection when client does not supply a certificate.
        //
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "", "");
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        d["IceSSL.TrustOnly"] = "!CN=ca1.client";
        d["IceSSL.VerifyPeer"] = "0";
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        //
        // Rejection takes precedence (client).
        //
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty("IceSSL.TrustOnly", "ST=Florida;!CN=ca1.server;C=US");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        //
        // Rejection takes precedence (server).
        //
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        d["IceSSL.TrustOnly"] = "C=US;!CN=ca1.client;ST=Florida";

        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    cout << "ok" << endl;

    cout << "testing IceSSL.TrustOnly.Client... " << flush;
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty(
            "IceSSL.TrustOnly.Client",
            "C=US, ST=Florida, O=ZeroC,"
            "OU=Ice test infrastructure, emailAddress=info@zeroc.com, CN=ca1.server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        // Should have no effect.
        d["IceSSL.TrustOnly.Client"] =
            "C=US, ST=Florida, O=ZeroC, OU=Ice test infrastructure, emailAddress=info@zeroc.com,"
            "CN=ca1.server";

        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty(
            "IceSSL.TrustOnly.Client",
            "!C=US, ST=Florida, O=ZeroC,"
            "OU=Ice test infrastructure, emailAddress=info@zeroc.com, CN=ca1.server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        // Should have no effect.
        d["IceSSL.TrustOnly.Client"] = "!CN=ca1.client";
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty("IceSSL.TrustOnly.Client", "CN=ca1.client");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        initData.properties->setProperty("IceSSL.TrustOnly.Client", "!CN=ca1.client");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    cout << "ok" << endl;

    cout << "testing IceSSL.TrustOnly.Server... " << flush;
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        // Should have no effect.
        initData.properties->setProperty(
            "IceSSL.TrustOnly.Server",
            "C=US, ST=Florida, O=ZeroC, OU=Ice test infrastructure,"
            "emailAddress=info@zeroc.com,CN=ca1.client");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        d["IceSSL.TrustOnly.Server"] =
            "C=US, ST=Florida, O=ZeroC, OU=Ice test infrastructure, emailAddress=info@zeroc.com,"
            "CN=ca1.client";

        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        d["IceSSL.TrustOnly.Server"] =
            "!C=US, ST=Florida, O=ZeroC, OU=Ice test infrastructure, emailAddress=info@zeroc.com, CN=ca1.client";
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        // Should have no effect.
        initData.properties->setProperty("IceSSL.TrustOnly.Server", "!CN=ca1.server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        d["IceSSL.TrustOnly.Server"] = "CN=ca1.server";
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        d["IceSSL.TrustOnly.Server"] = "!CN=ca1.client";
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    cout << "ok" << endl;

    cout << "testing IceSSL.TrustOnly.Server.<AdapterName>... " << flush;
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        d["IceSSL.TrustOnly.Server.ServerAdapter"] =
            "C=US, ST=Florida, O=ZeroC, OU=Ice test infrastructure, emailAddress=info@zeroc.com,CN=ca1.client";
        d["IceSSL.TrustOnly.Server"] = "CN=bogus";
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        d["IceSSL.TrustOnly.Server.ServerAdapter"] =
            "!C=US, ST=Florida, O=ZeroC, OU=Ice test infrastructure, emailAddress=info@zeroc.com, CN=ca1.client";
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        d["IceSSL.TrustOnly.Server.ServerAdapter"] = "CN=bogus";
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch (const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "ca1/client", "ca1/ca1");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact{comm, factoryRef};
        Test::Properties d = createServerProps(defaultProps, p12, "ca1/server", "ca1/ca1");
        d["IceSSL.TrustOnly.Server.ServerAdapter"] = "!CN=bogus";
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    cout << "ok" << endl;
}
#endif

#if defined(ICE_USE_SCHANNEL) || defined(ICE_USE_OPENSSL)
void
testCrlRevocation(const string& factoryRef, const string& defaultDir, const Ice::PropertiesPtr& defaultProps, bool p12)
{
    cout << "testing certificate revocation using CRL... " << flush;
    CommunicatorPtr comm;
    InitializationData initData;

    // First test with non revoked certificate that include CRL distribution point
    initData.properties = createClientProps(defaultProps, p12, "", "ca3/ca3");
    // CLR file used by OpenSSL, OpenSSL doesn't check the CRL distribution points.
    initData.properties->setProperty("IceSSL.CertificateRevocationListFiles", "ca3/ca3.crl.pem");
    initData.properties->setProperty("IceSSL.RevocationCheck", "1");
    initData.properties->setProperty("IceSSL.RevocationCheckCacheOnly", "0");
    initData.properties->setProperty("IceSSL.VerifyPeer", "0");
    comm = initialize(initData);
    Test::ServerFactoryPrx fact{comm, factoryRef};

    Test::Properties d = createServerProps(defaultProps, p12, "ca3/server", "");
    d["IceSSL.VerifyPeer"] = "0";
    optional<Test::ServerPrx> server = fact->createServer(d);

    server->ice_ping();
    Ice::SSL::ConnectionInfoPtr info =
        dynamic_pointer_cast<Ice::SSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
    fact->destroyServer(server);
    comm->destroy();

    // Repeat with RevocationCheck=2 to check whole chain
    initData.properties = createClientProps(defaultProps, p12, "", "ca3/ca3");
    // CLR file used by OpenSSL, OpenSSL doesn't check the CRL distribution points.
    initData.properties->setProperty("IceSSL.CertificateRevocationListFiles", "ca3/ca3.crl.pem");
    initData.properties->setProperty("IceSSL.RevocationCheck", "2");
    initData.properties->setProperty("IceSSL.RevocationCheckCacheOnly", "0");
    initData.properties->setProperty("IceSSL.VerifyPeer", "0");
    comm = initialize(initData);
    fact = Test::ServerFactoryPrx{comm, factoryRef};

    d = createServerProps(defaultProps, p12, "ca3/server", "");
    d["IceSSL.VerifyPeer"] = "0";
    server = fact->createServer(d);

    server->ice_ping();
    info = dynamic_pointer_cast<Ice::SSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
    fact->destroyServer(server);
    comm->destroy();

    // Repeat with revoked certificate
    initData.properties = createClientProps(defaultProps, p12, "", "ca3/ca3");
    initData.properties->setProperty("IceSSL.RevocationCheck", "0");
    // CLR file used by OpenSSL, OpenSSL doesn't check the CRL distribution points.
    initData.properties->setProperty("IceSSL.CertificateRevocationListFiles", "ca3/ca3.crl.pem");
    comm = initialize(initData);
    fact = Test::ServerFactoryPrx{comm, factoryRef};

    d = createServerProps(defaultProps, p12, "ca3/server_revoked", "");
    d["IceSSL.VerifyPeer"] = "0";
    server = fact->createServer(d);

    // Revoked certificate is accepted because IceSSL.RevocationCheck=0 disable revocation checks
    server->ice_ping();
    info = dynamic_pointer_cast<Ice::SSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
    fact->destroyServer(server);
    comm->destroy();

    // Repeat enabling revocation checks
    initData.properties = createClientProps(defaultProps, p12, "", "ca3/ca3");
    initData.properties->setProperty("IceSSL.RevocationCheck", "1");
    initData.properties->setProperty("IceSSL.RevocationCheckCacheOnly", "0");
    initData.properties->setProperty("IceSSL.CertificateRevocationListFiles", "ca3/ca3.crl.pem");
    initData.properties->setProperty("IceSSL.VerifyPeer", "0");
    comm = initialize(initData);
    fact = Test::ServerFactoryPrx{comm, factoryRef};

    d = createServerProps(defaultProps, p12, "ca3/server_revoked", "");
    d["IceSSL.VerifyPeer"] = "0";
    server = fact->createServer(d);

    try
    {
        server->ice_ping();
        test(false);
    }
    catch (const Ice::SecurityException&)
    {
        // expected
    }

    fact->destroyServer(server);
    comm->destroy();

    // Test with ca3/i1/server.p12 only the intermediate CA cert is revoked
    const char* certificates[] = {"/ca3/i1/server.p12", nullptr};
    ImportCerts import{defaultDir, certificates};

    initData.properties = createClientProps(defaultProps, p12, "", "ca3/ca3");
    initData.properties->setProperty("IceSSL.RevocationCheck", "2");
    initData.properties->setProperty("IceSSL.RevocationCheckCacheOnly", "0");
    // CLR file used by OpenSSL, OpenSSL doesn't check the CRL distribution points.
    initData.properties->setProperty("IceSSL.CertificateRevocationListFiles", "ca3/i1/i1.crl.pem");
    initData.properties->setProperty("IceSSL.VerifyPeer", "0");
    comm = initialize(initData);
    fact = Test::ServerFactoryPrx{comm, factoryRef};

    d = createServerProps(defaultProps, p12, "ca3/i1/server", "");
    d["IceSSL.VerifyPeer"] = "0";
    server = fact->createServer(d);

    try
    {
        server->ice_ping();
        test(false);
    }
    catch (const Ice::SecurityException&)
    {
        // expected
    }

    fact->destroyServer(server);
    comm->destroy();

    // Repeat checking only the end cert
    initData.properties = createClientProps(defaultProps, p12, "", "ca3/ca3");
    initData.properties->setProperty("IceSSL.RevocationCheck", "1");
    initData.properties->setProperty("IceSSL.RevocationCheckCacheOnly", "0");
    // CLR file used by OpenSSL, OpenSSL doesn't check the CRL distribution points.
    initData.properties->setProperty("IceSSL.CertificateRevocationListFiles", "ca3/i1/i1.crl.pem");
    initData.properties->setProperty("IceSSL.VerifyPeer", "0");

    comm = initialize(initData);
    fact = Test::ServerFactoryPrx{comm, factoryRef};

    d = createServerProps(defaultProps, p12, "ca3/i1/server", "");
    d["IceSSL.VerifyPeer"] = "0";
    server = fact->createServer(d);

    server->ice_ping();
    info = dynamic_pointer_cast<Ice::SSL::ConnectionInfo>(server->ice_getConnection()->getInfo());

    fact->destroyServer(server);
    comm->destroy();
    import.cleanup();

    cout << "ok" << endl;
}
#else
void
testCrlRevocation(const string&, const string&, const Ice::PropertiesPtr&, bool)
{
}
#endif

#if defined(ICE_USE_SCHANNEL) || defined(ICE_USE_SECURE_TRANSPORT_MACOS)
void
testOcspRevocation(const string& factoryRef, const string& defaultDir, const Ice::PropertiesPtr& defaultProps, bool p12)
{
    cout << "testing certificate revocation using OCSP... " << flush;
    CommunicatorPtr comm;
    InitializationData initData;

    // First test with non revoked certificate that include AIA info
    initData.properties = createClientProps(defaultProps, p12, "", "ca4/ca4");
    initData.properties->setProperty("IceSSL.RevocationCheck", "1");
    initData.properties->setProperty("IceSSL.RevocationCheckCacheOnly", "0");
    initData.properties->setProperty("IceSSL.VerifyPeer", "0");
    comm = initialize(initData);
    Test::ServerFactoryPrx fact{comm, factoryRef};

    Test::Properties d = createServerProps(defaultProps, p12, "ca4/server", "");
    d["IceSSL.VerifyPeer"] = "0";
    optional<Test::ServerPrx> server = fact->createServer(d);

    server->ice_ping();
    Ice::SSL::ConnectionInfoPtr info =
        dynamic_pointer_cast<Ice::SSL::ConnectionInfo>(server->ice_getConnection()->getInfo());

    fact->destroyServer(server);
    comm->destroy();

    // Now check with a revoked certificate and RevocationCheck=0 to disable revocation checks
#    ifndef ICE_USE_SECURE_TRANSPORT
    // With secure transport there is no reliable way to disable revocation checks
    initData.properties = createClientProps(defaultProps, p12, "", "ca4/ca4");
    initData.properties->setProperty("IceSSL.RevocationCheck", "0");
    comm = initialize(initData);
    fact = Test::ServerFactoryPrx{comm, factoryRef};

    d = createServerProps(defaultProps, p12, "ca4/server_revoked", "");
    d["IceSSL.VerifyPeer"] = "0";
    server = fact->createServer(d);
    server->ice_ping();
    fact->destroyServer(server);
    comm->destroy();
#    endif

    // Repeat with RevocationCheck=2 to check whole chain
    initData.properties = createClientProps(defaultProps, p12, "", "ca4/ca4");
    initData.properties->setProperty("IceSSL.RevocationCheck", "2");
    initData.properties->setProperty("IceSSL.RevocationCheckCacheOnly", "0");
    initData.properties->setProperty("IceSSL.VerifyPeer", "0");
    comm = initialize(initData);
    fact = Test::ServerFactoryPrx{comm, factoryRef};

    d = createServerProps(defaultProps, p12, "ca4/server", "");
    d["IceSSL.VerifyPeer"] = "0";
    server = fact->createServer(d);

    server->ice_ping();
    info = dynamic_pointer_cast<Ice::SSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
    fact->destroyServer(server);
    comm->destroy();

    // Test with server signed by ca4 intermediate, the intermediate CA cert is revoked.
    const char* certificates[] = {"/ca4/i1/server.p12", nullptr};
    ImportCerts import{defaultDir, certificates};
    initData.properties = createClientProps(defaultProps, p12, "", "ca4/ca4");
    initData.properties->setProperty("IceSSL.RevocationCheck", "2");
    initData.properties->setProperty("IceSSL.RevocationCheckCacheOnly", "0");
    initData.properties->setProperty("IceSSL.VerifyPeer", "0");
    comm = initialize(initData);
    fact = Test::ServerFactoryPrx{comm, factoryRef};

    d = createServerProps(defaultProps, p12, "ca4/i1/server", "");
    d["IceSSL.VerifyPeer"] = "0";
    server = fact->createServer(d);

    try
    {
        server->ice_ping();
        test(false);
    }
    catch (Ice::SecurityException&)
    {
        // expected
    }
    fact->destroyServer(server);
    comm->destroy();

    // Repeat with RevocationCheck=1 to only check the end cert
#    ifndef ICE_USE_SECURE_TRANSPORT
    // SecureTransport always check the whole chain for revocation
    initData.properties = createClientProps(defaultProps, p12, "", "ca4/ca4");
    initData.properties->setProperty("IceSSL.RevocationCheck", "1");
    initData.properties->setProperty("IceSSL.RevocationCheckCacheOnly", "0");
    initData.properties->setProperty("IceSSL.VerifyPeer", "0");

    comm = initialize(initData);
    fact = Test::ServerFactoryPrx{comm, factoryRef};

    d = createServerProps(defaultProps, p12, "ca4/i1/server", "");
    d["IceSSL.VerifyPeer"] = "0";
    server = fact->createServer(d);

    server->ice_ping();
    info = dynamic_pointer_cast<Ice::SSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
    fact->destroyServer(server);
    comm->destroy();
#    endif

    // Repeat with a certificate that is unknow for the OCSP responder
    initData.properties = createClientProps(defaultProps, p12, "", "ca4/ca4");
    initData.properties->setProperty("IceSSL.RevocationCheck", "1");
    initData.properties->setProperty("IceSSL.RevocationCheckCacheOnly", "0");
    initData.properties->setProperty("IceSSL.VerifyPeer", "0");

    comm = initialize(initData);
    fact = Test::ServerFactoryPrx{comm, factoryRef};

    d = createServerProps(defaultProps, p12, "ca4/server_unknown", "");
    d["IceSSL.VerifyPeer"] = "0";
    server = fact->createServer(d);

    try
    {
        server->ice_ping();
        test(false);
    }
    catch (Ice::SecurityException&)
    {
        // expected
    }
    fact->destroyServer(server);
    comm->destroy();

    import.cleanup();

    cout << "ok" << endl;
}
#else
void
testOcspRevocation(const string&, const string&, const Ice::PropertiesPtr&, bool)
{
    // Not supported with OpenSSL
}
#endif

void
testSystemCAs(const Ice::PropertiesPtr& defaultProps)
{
    cout << "testing system CAs... " << flush;
    //
    // Retry a few times in case there are connectivity problems with demo.zeroc.com.
    //
    const int retryMax = 5;
    const int retryDelay = 1000;

    {
        int retryCount = 0;
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, false);
        initData.properties->setProperty("IceSSL.DefaultDir", "");
        initData.properties->setProperty("IceSSL.CheckCertName", "2");
        initData.pluginFactories = {Ice::wsPluginFactory()};
        CommunicatorPtr comm = initialize(initData);
        Ice::ObjectPrx p(comm, "Glacier2/router:wss -p 443 -h zeroc.com -r /demo-proxy/chat/glacier2");
        while (true)
        {
            try
            {
                p->ice_ping();
                test(false);
            }
            catch (const Ice::SecurityException&)
            {
                // Expected, by default we don't check for system CAs.
                break;
            }
            catch (const Ice::LocalException& ex)
            {
                if ((dynamic_cast<const Ice::ConnectTimeoutException*>(&ex)) ||
                    (dynamic_cast<const Ice::SocketException*>(&ex)) || (dynamic_cast<const Ice::DNSException*>(&ex)))
                {
                    if (++retryCount < retryMax)
                    {
                        cout << "retrying... " << flush;
                        this_thread::sleep_for(chrono::milliseconds(retryDelay));
                        continue;
                    }
                }

                cerr << "warning: unable to connect to demo.zeroc.com to check system CA:\n" << ex << endl;
                break;
            }
        }
        comm->destroy();
    }

    {
        int retryCount = 0;
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, false);
        initData.properties->setProperty("IceSSL.DefaultDir", "");
        initData.properties->setProperty("IceSSL.UsePlatformCAs", "1");
        initData.properties->setProperty("IceSSL.CheckCertName", "2");
        initData.pluginFactories = {Ice::wsPluginFactory()};
        CommunicatorPtr comm = initialize(initData);
        Ice::ObjectPrx p(comm, "Glacier2/router:wss -p 443 -h zeroc.com -r /demo-proxy/chat/glacier2");
        while (true)
        {
            try
            {
                p->ice_ping();
                break;
            }
            catch (const Ice::LocalException& ex)
            {
                if ((dynamic_cast<const Ice::ConnectTimeoutException*>(&ex)) ||
                    (dynamic_cast<const Ice::SocketException*>(&ex)) || (dynamic_cast<const Ice::DNSException*>(&ex)))
                {
                    if (++retryCount < retryMax)
                    {
                        cout << "retrying... " << flush;
                        this_thread::sleep_for(chrono::milliseconds(retryDelay));
                        continue;
                    }
                }

                cerr << "warning: unable to connect to demo.zeroc.com to check system CA:\n" << ex << endl;
                break;
            }
        }
        comm->destroy();
    }
    cout << "ok" << endl;
}

Test::ServerFactoryPrx
#if !defined(__APPLE__) || TARGET_OS_IPHONE == 0
allTests(Test::TestHelper* helper, const string& defaultDir, bool p12)
#else
allTests(Test::TestHelper* helper, const string& defaultDir, bool p12)
#endif
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    string factory = "factory:" + helper->getTestEndpoint("tcp");

    string defaultHost = communicator->getProperties()->getIceProperty("Ice.Default.Host");
    Ice::PropertiesPtr defaultProps = communicator->getProperties()->clone();
    defaultProps->setProperty("IceSSL.DefaultDir", defaultDir);

    testCertificateVerification(factory, defaultHost, defaultProps, p12);
    testFindCert(factory, defaultDir, defaultProps, p12);
    testExpiredCert(factory, defaultProps, p12);
    testNotYerValidCert(factory, defaultProps, p12);
    testCertificateChains(factory, defaultDir, defaultProps, p12);
    testCAsDirectory(factory, defaultDir, defaultProps, p12);
    testMutlipleCACerts(factory, defaultProps, p12);
    testDerCertificates(factory, defaultProps, p12);
    testTrustOnly(factory, defaultProps, p12);
    testCrlRevocation(factory, defaultDir, defaultProps, p12);
    testOcspRevocation(factory, defaultDir, defaultProps, p12);
    testSystemCAs(defaultProps);

    return Test::ServerFactoryPrx{communicator, factory};
}
