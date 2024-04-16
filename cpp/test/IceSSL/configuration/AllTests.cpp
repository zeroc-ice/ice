//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <thread>
#include <vector>

#include "Ice/UniqueRef.h"

#if defined(__APPLE__)
#    include <sys/sysctl.h>
#    if TARGET_OS_IPHONE != 0
#        include "IceSSL/SecureTransportUtil.h" // For loadCertificateChain
#    endif
#endif

#if defined(__APPLE__)
#    define ICE_USE_SECURE_TRANSPORT 1
#    if defined(__APPLE__) && TARGET_OS_IPHONE != 0
#        define ICE_USE_SECURE_TRANSPORT_IOS 1
#    else
#        define ICE_USE_SECURE_TRANSPORT_MACOS 1
#    endif
#elif defined(_WIN32)
#    define ICE_USE_SCHANNEL 1
// We need to include windows.h before wincrypt.h.
// clang-format off
#    ifndef NOMINMAX
#        define NOMINMAX
#    endif
#    include <windows.h>
#    include <wincrypt.h>
// clang-format on

#else
#    define ICE_USE_OPENSSL 1
#endif

using namespace std;
using namespace Ice;

string
toHexString(vector<uint8_t> data)
{
    ostringstream os;
    for (vector<uint8_t>::const_iterator i = data.begin(); i != data.end();)
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
            //
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
            string resolved;
            if (IceSSL::checkPath(certificates[i], defaultDir, false, resolved))
            {
                IceInternal::UniqueRef<CFArrayRef> certs(
                    IceSSL::SecureTransport::loadCertificateChain(resolved, "", "", "", "password"));
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

                // query = CFDictionaryCreateMutable(0, 0, &kCFTypeDictionaryKeyCallBacks,
                // &kCFTypeDictionaryValueCallBacks); CFDictionarySetValue(query, kSecClass, kSecClassCertificate);
                // CFDictionarySetValue(query, kSecReturnRef, kCFBooleanTrue);
                // CFDictionarySetValue(query, kSecMatchLimit, kSecMatchLimitAll);
                // CFArrayRef array = 0;
                // err = SecItemCopyMatching(query, (CFTypeRef*)&array);
                // printf("Certificates\n");
                // for(int i = 0; i < CFArrayGetCount(array); ++i)
                // {
                //     printf("Cert %d: %s\n", i, (new
                //     IceSSL::Certificate((SecCertificateRef)CFArrayGetValueAtIndex(array, i)))->toString().c_str());
                // }
            }
        }
        // Nothing to do.
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
    PropertiesPtr result = createProperties();
    result->setProperty("IceSSL.DefaultDir", defaultProps->getProperty("IceSSL.DefaultDir"));
    result->setProperty("Ice.Default.Host", defaultProps->getProperty("Ice.Default.Host"));
    if (!defaultProps->getProperty("Ice.IPv6").empty())
    {
        result->setProperty("Ice.IPv6", defaultProps->getProperty("Ice.IPv6"));
    }
    if (p12)
    {
        result->setProperty("IceSSL.Password", "password");
    }
    // result->setProperty("IceSSL.Trace.Security", "1");
    // result->setProperty("Ice.Trace.Network", "3");
#ifdef ICE_USE_SECURE_TRANSPORT
    ostringstream keychainName;
    keychainName << "../certs/keychain/client" << keychainN++ << ".keychain";
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
    result["IceSSL.DefaultDir"] = defaultProps->getProperty("IceSSL.DefaultDir");
    result["Ice.Default.Host"] = defaultProps->getProperty("Ice.Default.Host");
    if (!defaultProps->getProperty("Ice.IPv6").empty())
    {
        result["Ice.IPv6"] = defaultProps->getProperty("Ice.IPv6");
    }
    if (p12)
    {
        result["IceSSL.Password"] = "password";
    }
    // result["Ice.Trace.Network"] = "3";
    // result["IceSSL.Trace.Security"] = "1";
#ifdef ICE_USE_SECURE_TRANSPORT
    ostringstream keychainName;
    keychainName << "../certs/keychain/server" << keychainN << ".keychain";
    result["IceSSL.Keychain"] = keychainName.str();
    result["IceSSL.KeychainPassword"] = "password";
#endif
    return result;
}

static Test::Properties
createServerProps(const Ice::PropertiesPtr& defaultProps, bool p12, const string& cert, const string& ca)
{
    Test::Properties d;
    d = createServerProps(defaultProps, p12);
    if (!ca.empty())
    {
        d["IceSSL.CAs"] = ca + ".pem";
    }

    if (!cert.empty())
    {
        if (p12)
        {
            d["IceSSL.CertFile"] = cert + ".p12";
        }
        else
        {
            d["IceSSL.CertFile"] = cert + "_pub.pem";
            d["IceSSL.KeyFile"] = cert + "_priv.pem";
        }
    }
    return d;
}

static PropertiesPtr
createClientProps(const Ice::PropertiesPtr& defaultProps, bool p12, const string& cert, const string& ca)
{
    Ice::PropertiesPtr properties;

    properties = createClientProps(defaultProps, p12);
    if (!ca.empty())
    {
        properties->setProperty("IceSSL.CAs", ca + ".pem");
    }

    if (!cert.empty())
    {
        if (p12)
        {
            properties->setProperty("IceSSL.CertFile", cert + ".p12");
        }
        else
        {
            properties->setProperty("IceSSL.CertFile", cert + "_pub.pem");
            properties->setProperty("IceSSL.KeyFile", cert + "_priv.pem");
        }
    }
    return properties;
}

void
verify(const IceSSL::CertificatePtr& cert, const IceSSL::CertificatePtr& ca)
{
    cerr << "Verify signature: ";
    if (cert->verify(ca))
    {
        cerr << " VALID";
    }
    else
    {
        cerr << " INVALID";
    }
    cerr << endl;
}

Test::ServerFactoryPrx
#if !defined(__APPLE__) || TARGET_OS_IPHONE == 0
allTests(Test::TestHelper* helper, const string& testDir, bool p12)
#else
allTests(Test::TestHelper* helper, const string& /*testDir*/, bool p12)
#endif
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    bool isCatalinaOrGreater = false;
    bool isIOS13OrGreater = false;
#ifdef __APPLE__
    vector<char> s(256);
    size_t size = s.size();
    int ret = sysctlbyname("kern.osrelease", &s[0], &size, nullptr, 0);
    if (ret == 0)
    {
        // version format is x.y.z
        size_t first = string(&s[0]).find_first_of(".");
        int majorVersion = atoi(string(&s[0]).substr(0, first).c_str());
#    if TARGET_OS_IPHONE == 0
        isCatalinaOrGreater = majorVersion >= 19;
#    else
        isIOS13OrGreater = majorVersion >= 18;
#    endif
    }
#endif
    string factoryRef = "factory:" + helper->getTestEndpoint("tcp");
    Test::ServerFactoryPrx factory(communicator, factoryRef);

    string defaultHost = communicator->getProperties()->getProperty("Ice.Default.Host");
#if !defined(__APPLE__) || TARGET_OS_IPHONE == 0
    string defaultDir = testDir + "/../certs";
#else
    string defaultDir = "certs";
#endif

    Ice::PropertiesPtr defaultProps = communicator->getProperties()->clone();
    defaultProps->setProperty("IceSSL.DefaultDir", defaultDir);

#ifdef _WIN32
    string sep = ";";
#else
    string sep = ":";
#endif

    IceSSL::ConnectionInfoPtr info;

    cout << "testing certificate verification... " << flush;
    {
        Test::Properties d;
        optional<Test::ServerPrx> server;
        optional<Test::ServerFactoryPrx> fact;
        CommunicatorPtr comm;
        InitializationData initData;
        //
        // Test IceSSL.VerifyPeer=0. Client does not have a certificate,
        // and doesn't trust the server certificate.
        //

        initData.properties = createClientProps(defaultProps, p12, "", "");
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);

        d = createServerProps(defaultProps, p12, "s_rsa_ca1", "");
        d["IceSSL.VerifyPeer"] = "0";
        server = fact->createServer(d);
        try
        {
            server->noCert();
            test(!dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo())->verified);
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();

        //
        // Test IceSSL.VerifyPeer=0. Client does not have a certificate,
        // but it still verifies the server's.
        //
        initData.properties = createClientProps(defaultProps, p12, "", "cacert1");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);

        d = createServerProps(defaultProps, p12, "s_rsa_ca1", "");
        d["IceSSL.VerifyPeer"] = "0";
        server = fact->createServer(d);
        try
        {
            server->noCert();
            test(dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo())->verified);
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        //
        // Test IceSSL.VerifyPeer=1. Client does not have a certificate.
        //
        d = createServerProps(defaultProps, p12, "s_rsa_ca1", "");
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

        //
        // Test IceSSL.VerifyPeer=2. This should fail because the client
        // does not supply a certificate.
        //
        d = createServerProps(defaultProps, p12, "s_rsa_ca1", "");
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
        //
        // Test IceSSL.VerifyPeer=1. Client has a certificate.
        //
        // Provide "cacert1" to the client to verify the server
        // certificate (without this the client connection wouldn't be
        // able to provide the certificate chain).
        //
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");

        comm = initialize(initData);
        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);

        d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
        d["IceSSL.VerifyPeer"] = "1";
        server = fact->createServer(d);
        try
        {
#if defined(_WIN32) && defined(ICE_USE_OPENSSL)
            IceSSL::CertificatePtr clientCert = IceSSL::OpenSSL::Certificate::load(defaultDir + "/c_rsa_ca1_pub.pem");
#else
            IceSSL::CertificatePtr clientCert = IceSSL::Certificate::load(defaultDir + "/c_rsa_ca1_pub.pem");
#endif
            server->checkCert(clientCert->getSubjectDN().toString(), clientCert->getIssuerDN().toString());

            //
            // Validate that we can get the connection info. Validate
            // that the certificates have the same DN.
            //
            // Validate some aspects of the Certificate class.
            //
#if defined(_WIN32) && defined(ICE_USE_OPENSSL)
            IceSSL::CertificatePtr serverCert = IceSSL::OpenSSL::Certificate::load(defaultDir + "/s_rsa_ca1_pub.pem");
            test(Ice::targetEqualTo(IceSSL::OpenSSL::Certificate::decode(serverCert->encode()), serverCert));
#else
            IceSSL::CertificatePtr serverCert = IceSSL::Certificate::load(defaultDir + "/s_rsa_ca1_pub.pem");
            test(Ice::targetEqualTo(IceSSL::Certificate::decode(serverCert->encode()), serverCert));
#endif
            test(Ice::targetEqualTo(serverCert, serverCert));
#if !defined(__APPLE__) || TARGET_OS_IPHONE == 0
            test(serverCert->checkValidity());

            test(!serverCert->checkValidity(std::chrono::system_clock::time_point()));
#endif

#if defined(_WIN32) && defined(ICE_USE_OPENSSL)
            IceSSL::CertificatePtr caCert = IceSSL::OpenSSL::Certificate::load(defaultDir + "/cacert1.pem");
            IceSSL::CertificatePtr caCert2 = IceSSL::OpenSSL::Certificate::load(defaultDir + "/cacert2.pem");
#else
            IceSSL::CertificatePtr caCert = IceSSL::Certificate::load(defaultDir + "/cacert1.pem");
            IceSSL::CertificatePtr caCert2 = IceSSL::Certificate::load(defaultDir + "/cacert2.pem");
#endif
            test(Ice::targetEqualTo(caCert, caCert));
#if !defined(__APPLE__) || TARGET_OS_IPHONE == 0
            test(caCert->checkValidity());

            test(!caCert->checkValidity(std::chrono::system_clock::time_point()));
#endif

            test(!serverCert->verify(serverCert));
            test(serverCert->verify(caCert));
            test(!serverCert->verify(caCert2));
            test(caCert->verify(caCert));

            info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
            test(info->certs.size() == 2);
            test(info->verified);
            test(info->errorCode == IceSSL::TrustError::NoError);

            test(Ice::targetEqualTo(caCert, info->certs[1]));
            test(Ice::targetEqualTo(serverCert, info->certs[0]));

            test(!Ice::targetEqualTo(serverCert, info->certs[1]));
            test(!Ice::targetEqualTo(caCert, info->certs[0]));

#if !defined(__APPLE__) || TARGET_OS_IPHONE == 0
            test(info->certs[0]->checkValidity() && info->certs[1]->checkValidity());

            test(
                !info->certs[0]->checkValidity(std::chrono::system_clock::time_point()) &&
                !info->certs[1]->checkValidity(std::chrono::system_clock::time_point()));
#endif

            test(
                info->certs.size() == 2 && info->certs[0]->getSubjectDN() == serverCert->getSubjectDN() &&
                info->certs[0]->getIssuerDN() == serverCert->getIssuerDN());
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);

        //
        // Test IceSSL.VerifyPeer=2. Client has a certificate.
        //
        d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
        d["IceSSL.VerifyPeer"] = "2";
        server = fact->createServer(d);
        try
        {
#if defined(_WIN32) && defined(ICE_USE_OPENSSL)
            IceSSL::CertificatePtr clientCert = IceSSL::OpenSSL::Certificate::load(defaultDir + "/c_rsa_ca1_pub.pem");
#else
            IceSSL::CertificatePtr clientCert = IceSSL::Certificate::load(defaultDir + "/c_rsa_ca1_pub.pem");
#endif
            server->checkCert(clientCert->getSubjectDN().toString(), clientCert->getIssuerDN().toString());
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);

        comm->destroy();

        //
        // Test IceSSL.VerifyPeer=1. This should fail because the client doesn't
        // trust the server's CA.
        //
        initData.properties = createClientProps(defaultProps, p12, "", "");
        initData.properties->setProperty("IceSSL.VerifyPeer", "1");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
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

        //
        // Test IceSSL.VerifyPeer=1. This should fail because the server doesn't
        // trust the client's CA.
        //
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca2", "");
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        d = createServerProps(defaultProps, p12, "s_rsa_ca1", "");
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

        //
        // This should succeed because the self signed certificate used by the server is
        // trusted.
        //
        initData.properties = createClientProps(defaultProps, p12, "", "cacert2");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        d = createServerProps(defaultProps, p12, "cacert2", "");
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

        //
        // This should fail because the self signed certificate used by the server is not
        // trusted.
        //
        initData.properties = createClientProps(defaultProps, p12, "", "");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        d = createServerProps(defaultProps, p12, "cacert2", "");
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

        //
        // Verify that IceSSL.CheckCertName has no effect in a server.
        //
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
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

        //
        // Test Hostname verification only when Ice.DefaultHost is 127.0.0.1
        // as that is the IP address used in the test certificates.
        //
        if (defaultHost == "127.0.0.1")
        {
            //
            // Test using localhost as target host
            //
            Ice::PropertiesPtr props = defaultProps->clone();
            props->setProperty("Ice.Default.Host", "localhost");

            //
            // Target host matches the certificate DNS altName
            //
            initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
            initData.properties->setProperty("IceSSL.CheckCertName", "1");
            initData.properties->setProperty("IceSSL.VerifyPeer", "0");
            comm = initialize(initData);

            fact = Test::ServerFactoryPrx(comm, factoryRef);
            test(fact);
            d = createServerProps(props, p12, "s_rsa_ca1_cn1", "cacert1");
            server = fact->createServer(d);

            info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
            test(info->verified);
            test(info->host == "localhost");
            test(info->errorCode == IceSSL::TrustError::NoError);

            fact->destroyServer(server);
            comm->destroy();

            //
            // Target host does not match the certificate DNS altName
            //
            initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
            initData.properties->setProperty("IceSSL.CheckCertName", "1");
            initData.properties->setProperty("IceSSL.VerifyPeer", "0");
            comm = initialize(initData);

            fact = Test::ServerFactoryPrx(comm, factoryRef);
            test(fact);
            d = createServerProps(props, p12, "s_rsa_ca1_cn2", "cacert1");
            server = fact->createServer(d);

            info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
            test(!info->verified);
            test(info->errorCode == IceSSL::TrustError::HostNameMismatch);
            test(info->host == "localhost");

            fact->destroyServer(server);
            comm->destroy();

            //
            // Target host matches the certificate Common Name and the certificate does not
            // include a DNS altName
            //
            initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
            initData.properties->setProperty("IceSSL.CheckCertName", "1");
            initData.properties->setProperty("IceSSL.VerifyPeer", "0");
            comm = initialize(initData);

            fact = Test::ServerFactoryPrx(comm, factoryRef);
            test(fact);
            d = createServerProps(props, p12, "s_rsa_ca1_cn3", "cacert1");
            server = fact->createServer(d);

            info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
            if (isCatalinaOrGreater || isIOS13OrGreater)
            {
                test(!info->verified);
                test(info->errorCode == IceSSL::TrustError::HostNameMismatch);
            }
            else
            {
                test(info->verified);
            }
            test(info->host == "localhost");

            fact->destroyServer(server);
            comm->destroy();

            //
            // Target host does not match the certificate Common Name and the certificate does not
            // include a DNS altName
            //
            initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
            initData.properties->setProperty("IceSSL.CheckCertName", "1");
            initData.properties->setProperty("IceSSL.VerifyPeer", "0");
            comm = initialize(initData);

            fact = Test::ServerFactoryPrx(comm, factoryRef);
            test(fact);
            d = createServerProps(props, p12, "s_rsa_ca1_cn4", "cacert1");
            server = fact->createServer(d);

            info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
            test(!info->verified);
            test(info->errorCode == IceSSL::TrustError::HostNameMismatch);
            test(info->host == "localhost");

            fact->destroyServer(server);
            comm->destroy();

            //
            // Target host matches the certificate Common Name and the certificate has
            // a DNS altName that does not matches the target host
            //
            initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
            initData.properties->setProperty("IceSSL.CheckCertName", "1");
            initData.properties->setProperty("IceSSL.VerifyPeer", "0");
            comm = initialize(initData);

            fact = Test::ServerFactoryPrx(comm, factoryRef);
            test(fact);
            d = createServerProps(props, p12, "s_rsa_ca1_cn5", "cacert1");
            server = fact->createServer(d);

            info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
            test(!info->verified);
            test(info->errorCode == IceSSL::TrustError::HostNameMismatch);
            test(info->host == "localhost");

            fact->destroyServer(server);
            comm->destroy();

            //
            // Test using 127.0.0.1 as target host
            //

            //
            // Target host matches the certificate IP altName
            //
            initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
            initData.properties->setProperty("IceSSL.CheckCertName", "1");
            initData.properties->setProperty("IceSSL.VerifyPeer", "0");
            comm = initialize(initData);

            fact = Test::ServerFactoryPrx(comm, factoryRef);
            test(fact);
            d = createServerProps(defaultProps, p12, "s_rsa_ca1_cn6", "cacert1");
            server = fact->createServer(d);
            ;

            info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
            test(info->verified);
            test(info->errorCode == IceSSL::TrustError::NoError);
            test(info->host == "127.0.0.1");

            fact->destroyServer(server);
            comm->destroy();

            //
            // Target host does not match the certificate IP altName
            //
            initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
            initData.properties->setProperty("IceSSL.CheckCertName", "1");
            initData.properties->setProperty("IceSSL.VerifyPeer", "0");
            comm = initialize(initData);

            fact = Test::ServerFactoryPrx(comm, factoryRef);
            test(fact);
            d = createServerProps(defaultProps, p12, "s_rsa_ca1_cn7", "cacert1");
            server = fact->createServer(d);

            info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
            test(!info->verified);
            test(info->errorCode == IceSSL::TrustError::HostNameMismatch);
            test(info->host == "127.0.0.1");

            fact->destroyServer(server);
            comm->destroy();

            //
            // Target host is an IP addres that matches the CN and the certificate doesn't
            // include an IP altName.
            //
            // With SecureTransport implementation the target IP will match with the Certificate
            // CN and the test will pass. With other implementations IP address is only match with
            // the Certificate IP altName and the test will fail.
            //
            initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
            initData.properties->setProperty("IceSSL.CheckCertName", "1");
            initData.properties->setProperty("IceSSL.VerifyPeer", "0");
            comm = initialize(initData);

            fact = Test::ServerFactoryPrx(comm, factoryRef);
            test(fact);
            d = createServerProps(defaultProps, p12, "s_rsa_ca1_cn8", "cacert1");
            server = fact->createServer(d);

            info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
            test(!info->verified);
            test(info->errorCode == IceSSL::TrustError::HostNameMismatch);
            test(info->host == "127.0.0.1");
            fact->destroyServer(server);
            comm->destroy();
        }
    }
    cout << "ok" << endl;

#if !defined(ICE_USE_SECURE_TRANSPORT_IOS)
    cout << "testing certificate info... " << flush;
    {
        const char* certificates[] = {"/cacert1.pem", "/c_rsa_ca1_pub.pem", "/s_rsa_ca1_pub.pem", 0};

        const char* authorities[] = {
            "", // Self signed CA cert has not X509v3 Authority Key Identifier extension
            "02:FD:1B:E9:6F:4E:96:8F:0C:0E:99:61:8F:45:48:6B:2B:14:3C:31",
            "02:FD:1B:E9:6F:4E:96:8F:0C:0E:99:61:8F:45:48:6B:2B:14:3C:31",
            0};

        const char* subjects[] = {
            "02:FD:1B:E9:6F:4E:96:8F:0C:0E:99:61:8F:45:48:6B:2B:14:3C:31",
            "7F:4D:BF:80:65:E0:EE:A4:18:D5:6A:87:33:63:B3:76:7D:42:82:06",
            "EB:4A:7A:79:09:65:0F:45:40:E8:8C:E6:A8:27:74:34:AB:EA:AF:48",
            0};

        for (int i = 0; certificates[i] != 0; ++i)
        {
            IceSSL::CertificatePtr cert = IceSSL::Certificate::load(defaultDir + certificates[i]);
            test(toHexString(cert->getAuthorityKeyIdentifier()) == authorities[i]);
            test(toHexString(cert->getSubjectKeyIdentifier()) == subjects[i]);
        }

        IceSSL::CertificatePtr cert = IceSSL::Certificate::load(defaultDir + "/cacert1.pem");
        unsigned int keyUsage = cert->getKeyUsage();
        test(
            keyUsage ==
            (IceSSL::KEY_USAGE_DIGITAL_SIGNATURE | IceSSL::KEY_USAGE_KEY_CERT_SIGN | IceSSL::KEY_USAGE_CRL_SIGN));

        //  Digital Signature, Certificate Sign, CRL Sign
        cert = IceSSL::Certificate::load(defaultDir + "/cacert3.pem");
        keyUsage = cert->getKeyUsage();
        test(
            keyUsage ==
            (IceSSL::KEY_USAGE_DIGITAL_SIGNATURE | IceSSL::KEY_USAGE_KEY_CERT_SIGN | IceSSL::KEY_USAGE_CRL_SIGN));

        //  Digital Signature, Certificate Sign, CRL Sign
        cert = IceSSL::Certificate::load(defaultDir + "/cacert4.pem");
        keyUsage = cert->getKeyUsage();
        test(
            keyUsage ==
            (IceSSL::KEY_USAGE_DIGITAL_SIGNATURE | IceSSL::KEY_USAGE_KEY_CERT_SIGN | IceSSL::KEY_USAGE_CRL_SIGN));
    }

    {
        IceSSL::CertificatePtr cert = IceSSL::Certificate::load(defaultDir + "/rsa_ca1_none_pub.pem");
        unsigned int keyUsage = cert->getExtendedKeyUsage();
        test(keyUsage == 0);

        cert = IceSSL::Certificate::load(defaultDir + "/rsa_ca1_serverAuth_pub.pem");
        keyUsage = cert->getExtendedKeyUsage();
        test(keyUsage == IceSSL::EXTENDED_KEY_USAGE_SERVER_AUTH);

        cert = IceSSL::Certificate::load(defaultDir + "/rsa_ca1_clientAuth_pub.pem");
        keyUsage = cert->getExtendedKeyUsage();
        test(keyUsage == IceSSL::EXTENDED_KEY_USAGE_CLIENT_AUTH);

        cert = IceSSL::Certificate::load(defaultDir + "/rsa_ca1_codeSigning_pub.pem");
        keyUsage = cert->getExtendedKeyUsage();
        test(keyUsage == IceSSL::EXTENDED_KEY_USAGE_CODE_SIGNING);

        cert = IceSSL::Certificate::load(defaultDir + "/rsa_ca1_emailProtection_pub.pem");
        keyUsage = cert->getExtendedKeyUsage();
        test(keyUsage == IceSSL::EXTENDED_KEY_USAGE_EMAIL_PROTECTION);

        cert = IceSSL::Certificate::load(defaultDir + "/rsa_ca1_timeStamping_pub.pem");
        keyUsage = cert->getExtendedKeyUsage();
        test(keyUsage == IceSSL::EXTENDED_KEY_USAGE_TIME_STAMPING);

        cert = IceSSL::Certificate::load(defaultDir + "/rsa_ca1_ocspSigning_pub.pem");
        keyUsage = cert->getExtendedKeyUsage();
        test(keyUsage == IceSSL::EXTENDED_KEY_USAGE_OCSP_SIGNING);

        cert = IceSSL::Certificate::load(defaultDir + "/rsa_ca1_anyExtendedKeyUsage_pub.pem");
        keyUsage = cert->getExtendedKeyUsage();
        test(keyUsage == IceSSL::EXTENDED_KEY_USAGE_ANY_KEY_USAGE);
    }

    {
#    if !defined(__APPLE__) || TARGET_OS_IPHONE == 0
        vector<pair<int, string>> expectedAltNames;
        expectedAltNames.push_back(make_pair(7, "127.0.0.1"));
        expectedAltNames.push_back(make_pair(2, "client"));
        IceSSL::CertificatePtr cert = IceSSL::Certificate::load(defaultDir + "/c_rsa_ca1_pub.pem");
        test(cert->getSubjectAlternativeNames() == expectedAltNames);
        // Digital Signature, Non Repudiation, Key Encipherment
        unsigned int keyUsage = cert->getKeyUsage();
        test(
            keyUsage == (IceSSL::KEY_USAGE_DIGITAL_SIGNATURE | IceSSL::KEY_USAGE_NON_REPUDIATION |
                         IceSSL::KEY_USAGE_KEY_ENCIPHERMENT));

        expectedAltNames.clear();
        expectedAltNames.push_back(make_pair(7, "127.0.0.1"));
        expectedAltNames.push_back(make_pair(2, "server"));
        cert = IceSSL::Certificate::load(defaultDir + "/s_rsa_ca1_pub.pem");
        test(cert->getSubjectAlternativeNames() == expectedAltNames);
        keyUsage = cert->getKeyUsage();
        test(
            keyUsage == (IceSSL::KEY_USAGE_DIGITAL_SIGNATURE | IceSSL::KEY_USAGE_NON_REPUDIATION |
                         IceSSL::KEY_USAGE_KEY_ENCIPHERMENT));

        expectedAltNames.clear();
        expectedAltNames.push_back(make_pair(2, "localhost"));
        cert = IceSSL::Certificate::load(defaultDir + "/s_rsa_ca1_cn1_pub.pem");
        test(cert->getSubjectAlternativeNames() == expectedAltNames);

        expectedAltNames.clear();
        expectedAltNames.push_back(make_pair(2, "localhostXX"));
        cert = IceSSL::Certificate::load(defaultDir + "/s_rsa_ca1_cn2_pub.pem");
        test(cert->getSubjectAlternativeNames() == expectedAltNames);

        expectedAltNames.clear();
        expectedAltNames.push_back(make_pair(7, "127.0.0.1"));
        cert = IceSSL::Certificate::load(defaultDir + "/s_rsa_ca1_cn6_pub.pem");
        test(cert->getSubjectAlternativeNames() == expectedAltNames);

        expectedAltNames.clear();
        expectedAltNames.push_back(make_pair(7, "127.0.0.2"));
        cert = IceSSL::Certificate::load(defaultDir + "/s_rsa_ca1_cn7_pub.pem");
        test(cert->getSubjectAlternativeNames() == expectedAltNames);

        cert = IceSSL::Certificate::load(defaultDir + "/s_rsa_ca1_cn8_pub.pem");
        test(cert->getSubjectAlternativeNames().empty());

        expectedAltNames.clear();
        expectedAltNames.push_back(make_pair(7, "127.0.0.1"));
// IPv6 address parsing is not implemented with SChannel and OpenSSL IceSSL implementations
#        ifdef ICE_USE_SECURE_TRANSPORT
        expectedAltNames.push_back(make_pair(7, "0000:0000:0000:0000:0000:0000:0000:0001"));
#        endif
        cert = IceSSL::Certificate::load(defaultDir + "/s_rsa_ca1_cn9_pub.pem");
        test(cert->getSubjectAlternativeNames() == expectedAltNames);

        expectedAltNames.clear();
        expectedAltNames.push_back(make_pair(2, "host1"));
        expectedAltNames.push_back(make_pair(2, "host2"));
        cert = IceSSL::Certificate::load(defaultDir + "/s_rsa_ca1_cn10_pub.pem");
        test(cert->getSubjectAlternativeNames() == expectedAltNames);

        expectedAltNames.clear();
        expectedAltNames.push_back(make_pair(7, "127.0.0.1"));
        expectedAltNames.push_back(make_pair(7, "127.0.0.2"));
        expectedAltNames.push_back(make_pair(2, "host1"));
        expectedAltNames.push_back(make_pair(2, "host2"));
        cert = IceSSL::Certificate::load(defaultDir + "/s_rsa_ca1_cn11_pub.pem");
        test(cert->getSubjectAlternativeNames() == expectedAltNames);
#    endif
    }
    cout << "ok" << endl;
#endif

    cout << "testing certificate chains... " << flush;
    {
        const char* certificates[] = {"/s_rsa_cai2.p12", 0};
        ImportCerts import(defaultDir, certificates);

        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "", "");
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);

        //
        // The client can't verify the server certificate but it should
        // still provide it. "s_rsa_ca1" doesn't include the root so the
        // cert size should be 1.
        //
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "");
        d["IceSSL.VerifyPeer"] = "0";
        optional<Test::ServerPrx> server = fact->createServer(d);
        try
        {
            info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
            test(info->certs.size() == 1);
            test(!info->verified);
            test(info->errorCode == IceSSL::TrustError::PartialChain);
        }
        catch (const Ice::LocalException& ex)
        {
            import.cleanup();
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);

        //
        // Setting the CA for the server shouldn't change anything, it
        // shouldn't modify the cert chain sent to the client.
        //
        d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
        d["IceSSL.VerifyPeer"] = "0";
        server = fact->createServer(d);
        try
        {
            info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
#ifdef ICE_USE_OPENSSL
            test(info->certs.size() == 2); // TODO: Fix OpenSSL
            test(info->errorCode == IceSSL::TrustError::UntrustedRoot);
#else
            test(info->certs.size() == 1);
            test(info->errorCode == IceSSL::TrustError::PartialChain);
#endif
            test(!info->verified);
        }
        catch (const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            import.cleanup();
            test(false);
        }
        fact->destroyServer(server);

        //
        // The client can't verify the server certificate but should
        // still provide it. "s_rsa_wroot_ca1" includes the root so
        // the cert size should be 2.
        //
        if (p12)
        {
            d = createServerProps(defaultProps, p12, "s_rsa_wroot_ca1", "");
            d["IceSSL.VerifyPeer"] = "0";
            server = fact->createServer(d);
            try
            {
                info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
#if defined(ICE_USE_SCHANNEL)
                test(info->certs.size() == 1); // SChannel never sends the root certificate
                test(info->errorCode == IceSSL::TrustError::PartialChain);
#else
                test(info->certs.size() == 2);
                test(info->errorCode == IceSSL::TrustError::UntrustedRoot);
#endif
                test(!info->verified);
            }
            catch (const Ice::LocalException& ex)
            {
                cerr << ex << endl;
                import.cleanup();
                test(false);
            }
            fact->destroyServer(server);
        }
        comm->destroy();

        //
        // Now the client verifies the server certificate
        //
        initData.properties = createClientProps(defaultProps, p12, "", "cacert1");
        initData.properties->setProperty("IceSSL.VerifyPeer", "1");
        comm = initialize(initData);

        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);

        {
            d = createServerProps(defaultProps, p12, "s_rsa_ca1", "");
            d["IceSSL.VerifyPeer"] = "0";
            server = fact->createServer(d);
            try
            {
                info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
                test(info->certs.size() == 2);
                test(info->verified);
                test(info->errorCode == IceSSL::TrustError::NoError);
            }
            catch (const Ice::LocalException& ex)
            {
                cerr << ex << endl;
                import.cleanup();
                test(false);
            }
            fact->destroyServer(server);
        }
        comm->destroy();

        cout << "ok" << endl;
    }

#if defined(ICE_USE_OPENSSL) || defined(ICE_USE_SCHANNEL)
    cout << "testing certificate extensions... " << flush;
    {
        const string basicConstraints = "30:03:01:01:FF";

        const string subjectKeyIdentifier = "04:14:13:FA:72:67:FE:34:05:9A:C9:3E:61:D2:91:D6:BA:03:65:1B:8A:9A";

        const string authorityKeyIdentifier = "30:81:A2:80:14:13:FA:72:67:FE:34:05:9A:C9:3E:61:D2:91:D6:BA:03:65:"
                                              "1B:8A:9A:A1:7F:A4:7D:30:7B:31:0B:30:09:06:03:55:04:06:13:02:55:53:"
                                              "31:10:30:0E:06:03:55:04:08:0C:07:46:6C:6F:72:69:64:61:31:10:30:0E:"
                                              "06:03:55:04:07:0C:07:4A:75:70:69:74:65:72:31:0E:30:0C:06:03:55:04:"
                                              "0A:0C:05:5A:65:72:6F:43:31:0C:30:0A:06:03:55:04:0B:0C:03:49:63:65:"
                                              "31:0B:30:09:06:03:55:04:03:0C:02:43:41:31:1D:30:1B:06:09:2A:86:48:"
                                              "86:F7:0D:01:09:01:16:0E:69:6E:66:6F:40:7A:65:72:6F:63:2E:63:6F:6D:"
                                              "82:09:00:EA:2A:B7:FB:3B:A3:DF:5A";

        const string subjectAltName = "30:0B:82:09:7A:65:72:6F:63:2E:63:6F:6D";

        const string issuerAltName = "30:0B:82:09:7A:65:72:6F:63:2E:63:6F:6D";

        const string customExt412 = "0C:0B:43:75:73:74:6F:6D:20:64:61:74:61";

        const string customExt413 = "30:17:01:01:FF:0C:0E:4D:79:20:55:54:46:38:20:53:74:72:69:6E:67:02:02:03:FF";

        IceSSL::CertificatePtr cert = IceSSL::Certificate::load(defaultDir + "/cacert_custom.pem");
        vector<IceSSL::X509ExtensionPtr> extensions = cert->getX509Extensions();
        test(extensions.size() == 7);

        IceSSL::X509ExtensionPtr ext = cert->getX509Extension("2.5.29.19"); // Subject key identifier
        test(ext);
        test(toHexString(ext->getData()) == basicConstraints);
        test(ext->getOID() == "2.5.29.19");
        test(ext->isCritical() == false);

        ext = cert->getX509Extension("2.5.29.14"); // Subject key identifier
        test(ext);
        test(toHexString(ext->getData()) == subjectKeyIdentifier);
        test(ext->getOID() == "2.5.29.14");
        test(ext->isCritical() == false);

        ext = cert->getX509Extension("2.5.29.35"); // Authority key identifier
        test(ext);
        test(toHexString(ext->getData()) == authorityKeyIdentifier);
        test(ext->getOID() == "2.5.29.35");
        test(ext->isCritical() == false);

        ext = cert->getX509Extension("2.5.29.17"); // Subject alternative name
        test(ext);
        test(toHexString(ext->getData()) == subjectAltName);
        test(ext->getOID() == "2.5.29.17");
        test(ext->isCritical() == false);

        ext = cert->getX509Extension("2.5.29.18"); // Issuer alternative name
        test(ext);
        test(toHexString(ext->getData()) == issuerAltName);
        test(ext->getOID() == "2.5.29.18");
        test(ext->isCritical() == false);

        ext = cert->getX509Extension("1.2.3.412"); // Custom extension
        test(ext);
        test(toHexString(ext->getData()) == customExt412);
        test(ext->getOID() == "1.2.3.412");
        test(ext->isCritical() == false);

        ext = cert->getX509Extension("1.2.3.413"); // Custom extension
        test(ext);
        test(toHexString(ext->getData()) == customExt413);
        test(ext->getOID() == "1.2.3.413");
        test(ext->isCritical() == false);
    }
    cout << "ok" << endl;
#endif

    cout << "testing expired certificates... " << flush;
    {
        //
        // This should fail because the server's certificate is expired.
        //
#if !defined(__APPLE__) || TARGET_OS_IPHONE == 0
        {
            IceSSL::CertificatePtr cert = IceSSL::Certificate::load(defaultDir + "/s_rsa_ca1_exp_pub.pem");
            test(!cert->checkValidity());
        }
#endif

        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        CommunicatorPtr comm = initialize(initData);
        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1_exp", "cacert1");
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

        // repeat with VerifyPeer = 0
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        d = createServerProps(defaultProps, p12, "s_rsa_ca1_exp", "cacert1");
        server = fact->createServer(d);

        info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
        test(!info->verified);
        test(info->errorCode == IceSSL::TrustError::InvalidTime);

        fact->destroyServer(server);
        comm->destroy();

        //
        // This should fail because the client's certificate is expired.
        //
#if !defined(__APPLE__) || TARGET_OS_IPHONE == 0
        {
            IceSSL::CertificatePtr cert = IceSSL::Certificate::load(defaultDir + "/c_rsa_ca1_exp_pub.pem");
            test(!cert->checkValidity());
        }
#endif

        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1_exp", "cacert1");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
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
    }
    cout << "ok" << endl;

#ifdef ICE_USE_OPENSSL
    cout << "testing CA certificate directory... " << flush;
    {
        //
        // Don't specify CAs explicitly; we let OpenSSL find the CA
        // certificate in the default directory.
        //
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "");
        initData.properties->setProperty("IceSSL.CAs", defaultDir);
        CommunicatorPtr comm = initialize(initData);
        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "");
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
    }
    cout << "ok" << endl;
#endif

    cout << "testing multiple CA certificates... " << flush;
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacerts");
        CommunicatorPtr comm = initialize(initData);
        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca2", "cacerts");
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

    //
    // OpenSSL must use PEM certificate
    //
#if !defined(ICE_USE_OPENSSL)
    cout << "testing DER CA certificate... " << flush;
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "");
        initData.properties->setProperty("IceSSL.CAs", "cacert1.der");
        CommunicatorPtr comm = initialize(initData);
        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "");
        d["IceSSL.VerifyPeer"] = "2";
        d["IceSSL.CAs"] = "cacert1.der";
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
#endif

    cout << "testing IceSSL.TrustOnly... " << flush;
    //
    // iOS support only provides access to the CN of the certificate so we
    // can't check for other attributes
    //
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        initData.properties->setProperty(
            "IceSSL.TrustOnly",
            "C=US, ST=Florida, O=ZeroC\\, Inc.,"
            "OU=Ice, emailAddress=info@zeroc.com, CN=Server");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        initData.properties->setProperty(
            "IceSSL.TrustOnly",
            "!C=US, ST=Florida, O=ZeroC\\, Inc.,"
            "OU=Ice, emailAddress=info@zeroc.com, CN=Server");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        initData.properties->setProperty(
            "IceSSL.TrustOnly",
            "C=US, ST=Florida, O=\"ZeroC, Inc.\","
            "OU=Ice, emailAddress=info@zeroc.com, CN=Server");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
        d["IceSSL.TrustOnly"] = "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com,CN=Client";

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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
        d["IceSSL.TrustOnly"] = "!C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Client";

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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        initData.properties->setProperty("IceSSL.TrustOnly", "CN=Server");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        initData.properties->setProperty("IceSSL.TrustOnly", "!CN=Server");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
        d["IceSSL.TrustOnly"] = "CN=Client";
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
        d["IceSSL.TrustOnly"] = "!CN=Client";
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        initData.properties->setProperty("IceSSL.TrustOnly", "CN=Client");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
        d["IceSSL.TrustOnly"] = "CN=Server";
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        initData.properties->setProperty("IceSSL.TrustOnly", "C=Canada,CN=Server");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        initData.properties->setProperty("IceSSL.TrustOnly", "!C=Canada,CN=Server");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        initData.properties->setProperty("IceSSL.TrustOnly", "C=Canada;CN=Server");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        initData.properties->setProperty("IceSSL.TrustOnly", "!C=Canada;!CN=Server");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        initData.properties->setProperty("IceSSL.TrustOnly", "!CN=Server1"); // Should not match "Server"
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
        d["IceSSL.TrustOnly"] = "!CN=Client1"; // Should not match "Client"
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

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
        d["IceSSL.VerifyPeer"] = "0";
        d["IceSSL.TrustOnly"] = "CN=Client";
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

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
        d["IceSSL.TrustOnly"] = "!CN=Client";
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        initData.properties->setProperty("IceSSL.TrustOnly", "ST=Florida;!CN=Server;C=US");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
        d["IceSSL.TrustOnly"] = "C=US;!CN=Client;ST=Florida";

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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        initData.properties->setProperty(
            "IceSSL.TrustOnly.Client",
            "C=US, ST=Florida, O=ZeroC\\, Inc.,"
            "OU=Ice, emailAddress=info@zeroc.com, CN=Server");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
        // Should have no effect.
        d["IceSSL.TrustOnly.Client"] = "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com,"
                                       "CN=Server";

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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        initData.properties->setProperty(
            "IceSSL.TrustOnly.Client",
            "!C=US, ST=Florida, O=ZeroC\\, Inc.,"
            "OU=Ice, emailAddress=info@zeroc.com, CN=Server");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
        // Should have no effect.
        d["IceSSL.TrustOnly.Client"] = "!CN=Client";
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        initData.properties->setProperty("IceSSL.TrustOnly.Client", "CN=Client");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        initData.properties->setProperty("IceSSL.TrustOnly.Client", "!CN=Client");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        // Should have no effect.
        initData.properties->setProperty(
            "IceSSL.TrustOnly.Server",
            "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice,"
            "emailAddress=info@zeroc.com,CN=Client");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
        d["IceSSL.TrustOnly.Server"] = "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com,"
                                       "CN=Client";

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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
        d["IceSSL.TrustOnly.Server"] =
            "!C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Client";
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        // Should have no effect.
        initData.properties->setProperty("IceSSL.TrustOnly.Server", "!CN=Server");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
        d["IceSSL.TrustOnly.Server"] = "CN=Server";
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
        d["IceSSL.TrustOnly.Server"] = "!CN=Client";
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
        d["IceSSL.TrustOnly.Server.ServerAdapter"] =
            "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com,CN=Client";
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
        d["IceSSL.TrustOnly.Server.ServerAdapter"] =
            "!C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Client";
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
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
        initData.properties = createClientProps(defaultProps, p12, "c_rsa_ca1", "cacert1");
        CommunicatorPtr comm = initialize(initData);

        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);
        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
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

    {
#if defined(ICE_USE_SCHANNEL)
        cout << "testing IceSSL.FindCert... " << flush;
        const char* clientFindCertProperties[] = {
            "SUBJECTDN:'CN=Client, OU=Ice, O=\"ZeroC, Inc.\", L=Jupiter, S=Florida, C=US, E=info@zeroc.com'",
            "ISSUER:'ZeroC, Inc.' SUBJECT:Client SERIAL:02",
            "ISSUERDN:'CN=ZeroC Test CA 1, OU=Ice, O=\"ZeroC, Inc.\",L=Jupiter, S=Florida, C=US,E=info@zeroc.com' "
            "SUBJECT:Client",
            "THUMBPRINT:'8B D3 64 6B 9E 80 AE 56 08 05 0F C8 DE 9B B0 4B CC FD 4D 9C'",
            "SUBJECTKEYID:'7F 4D BF 80 65 E0 EE A4 18 D5 6A 87 33 63 B3 76 7D 42 82 06'",
            0};

        const char* serverFindCertProperties[] = {
            "SUBJECTDN:'CN=Server, OU=Ice, O=\"ZeroC, Inc.\", L=Jupiter, S=Florida, C=US, E=info@zeroc.com'",
            "ISSUER:'ZeroC, Inc.' SUBJECT:Server SERIAL:01",
            "ISSUERDN:'CN=ZeroC Test CA 1, OU=Ice, O=\"ZeroC, Inc.\", L=Jupiter, S=Florida, C=US,E=info@zeroc.com' "
            "SUBJECT:Server",
            "THUMBPRINT:'F2 EB 9D E7 A5 DB 32 2B AC 5B 4F 88 8F 5E 62 57 2E 2F 7B 8C'",
            "SUBJECTKEYID:'EB 4A 7A 79 09 65 0F 45 40 E8 8C E6 A8 27 74 34 AB EA AF 48'",
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

        const char* certificates[] = {"/s_rsa_ca1.p12", "/c_rsa_ca1.p12", 0};
        ImportCerts import(defaultDir, certificates);

        for (int i = 0; clientFindCertProperties[i] != 0; i++)
        {
            InitializationData initData;
            initData.properties = createClientProps(defaultProps, p12);
            initData.properties->setProperty("IceSSL.CAs", "cacert1.pem");
            initData.properties->setProperty("IceSSL.CertStore", "My");
            initData.properties->setProperty("IceSSL.CertStoreLocation", "CurrentUser");
            initData.properties->setProperty("IceSSL.FindCert", clientFindCertProperties[i]);
            //
            // Use TrustOnly to ensure the peer has pick the expected certificate.
            //
            initData.properties->setProperty("IceSSL.TrustOnly", "CN=Server");

            CommunicatorPtr comm = initialize(initData);

            optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
            test(fact);
            Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca1", "cacert1");
            d["IceSSL.CAs"] = "cacert1.pem";
            d["IceSSL.FindCert"] = serverFindCertProperties[i];
            //
            // Use TrustOnly to ensure the peer has pick the expected certificate.
            //
            d["IceSSL.TrustOnly"] = "CN=Client";

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

        //
        // These must fail because the search criteria does not match any certificates.
        //
        for (int i = 0; failFindCertProperties[i] != 0; i++)
        {
            InitializationData initData;
            initData.properties = createClientProps(defaultProps, p12);
            initData.properties->setProperty("IceSSL.CAs", "cacert1.pem");
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
            initData.properties->setProperty("IceSSL.CAs", "cacert1.pem");
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
#elif defined(ICE_USE_SECURE_TRANSPORT)
        cout << "testing IceSSL.FindCert... " << flush;
        const char* clientFindCertProperties[] = {
            //            "SUBJECT:Client",
            "LABEL:'Client'",
            "SUBJECTKEYID:'7F 4D BF 80 65 E0 EE A4 18 D5 6A 87 33 63 B3 76 7D 42 82 06'",
            "SERIAL:02",
            "SERIAL:02 LABEL:Client",
            0};

        const char* serverFindCertProperties[] = {
#    if !defined(__APPLE__) || TARGET_OS_IPHONE == 0
            // iOS match on Subject DN isn't supported by SecItemCopyMatch
            "SUBJECT:Server",
#    endif
            "LABEL:'Server'",
            "SUBJECTKEYID:'EB 4A 7A 79 09 65 0F 45 40 E8 8C E6 A8 27 74 34 AB EA AF 48'",
            "SERIAL:01",
            "SERIAL:01 LABEL:Server",
            0
        };

        const char* failFindCertProperties[] = {
            "nolabel",
            "unknownlabel:foo",
            "LABEL:",
#    if !defined(__APPLE__) || TARGET_OS_IPHONE == 0
            // iOS match on Subject DN isn't supported by SecItemCopyMatch
            "SUBJECT:ServerX",
#    endif
            "LABEL:'ServerX'",
            "SUBJECTKEYID:'a6 42 aa 17 04 41 86 56 67 e4 04 64 59 34 30 c7 4c 6b ef ff'",
            "SERIAL:04",
            "SERIAL:04 LABEL:Client",
            0
        };

        const char* certificates[] = {"/s_rsa_ca1.p12", "/c_rsa_ca1.p12", 0};
        ImportCerts import(defaultDir, certificates);

        for (int i = 0; clientFindCertProperties[i] != 0; i++)
        {
            InitializationData initData;
            initData.properties = createClientProps(defaultProps, p12);
            initData.properties->setProperty("IceSSL.CAs", "cacert1.pem");
            initData.properties->setProperty("IceSSL.Keychain", "../certs/Find.keychain");
            initData.properties->setProperty("IceSSL.KeychainPassword", "password");
            initData.properties->setProperty("IceSSL.FindCert", clientFindCertProperties[i]);
            //
            // Use TrustOnly to ensure the peer has pick the expected certificate.
            //
            initData.properties->setProperty("IceSSL.TrustOnly", "CN=Server");

            CommunicatorPtr comm = initialize(initData);

            optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
            test(fact);
            Test::Properties d = createServerProps(defaultProps, p12);
            d["IceSSL.CAs"] = "cacert1.pem";
            d["IceSSL.Keychain"] = "../certs/Find.keychain";
            d["IceSSL.KeychainPassword"] = "password";
            d["IceSSL.FindCert"] = serverFindCertProperties[i];

            //
            // Use TrustOnly to ensure the peer has pick the expected certificate.
            //
            d["IceSSL.TrustOnly"] = "CN=Client";

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

        for (int i = 0; failFindCertProperties[i] != 0; i++)
        {
            InitializationData initData;
            initData.properties = createClientProps(defaultProps, p12);
            initData.properties->setProperty("IceSSL.Keychain", "../certs/Find.keychain");
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
#endif
    }

    {
#if defined(ICE_USE_SCHANNEL) || defined(ICE_USE_OPENSSL)
        cout << "testing certificate revocation using CRL... " << flush;
        CommunicatorPtr comm;
        InitializationData initData;

        // First test with non revoked certificate that include CRL distribution point
        initData.properties = createClientProps(defaultProps, p12, "", "cacert3");
        // CLR file used by OpenSSL, OpenSSL doesn't check the CRL distribution points.
        initData.properties->setProperty("IceSSL.CertificateRevocationListFiles", "ca.crl.pem");
        initData.properties->setProperty("IceSSL.RevocationCheck", "1");
        initData.properties->setProperty("IceSSL.RevocationCheckCacheOnly", "0");
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");
        comm = initialize(initData);
        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);

        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca3", "");
        d["IceSSL.VerifyPeer"] = "0";
        optional<Test::ServerPrx> server = fact->createServer(d);

        server->ice_ping();
        info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
        test(info->errorCode == IceSSL::TrustError::NoError);
        test(info->verified);
        fact->destroyServer(server);
        comm->destroy();

        // Repeat with RevoactionCheck=2 to check whole chain
        initData.properties = createClientProps(defaultProps, p12, "", "cacert3");
        // CLR file used by OpenSSL, OpenSSL doesn't check the CRL distribution points.
        initData.properties->setProperty("IceSSL.CertificateRevocationListFiles", "ca.crl.pem");
        initData.properties->setProperty("IceSSL.RevocationCheck", "2");
        initData.properties->setProperty("IceSSL.RevocationCheckCacheOnly", "0");
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);

        d = createServerProps(defaultProps, p12, "s_rsa_ca3", "");
        d["IceSSL.VerifyPeer"] = "0";
        server = fact->createServer(d);

        server->ice_ping();
        info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
        test(info->errorCode == IceSSL::TrustError::NoError);
        test(info->verified);
        fact->destroyServer(server);
        comm->destroy();

        // Repeat with revoked certificate
        initData.properties = createClientProps(defaultProps, p12, "", "cacert3");
        initData.properties->setProperty("IceSSL.RevocationCheck", "0");
        // CLR file used by OpenSSL, OpenSSL doesn't check the CRL distribution points.
        initData.properties->setProperty("IceSSL.CertificateRevocationListFiles", "ca.crl.pem");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);

        d = createServerProps(defaultProps, p12, "s_rsa_ca3_revoked", "");
        d["IceSSL.VerifyPeer"] = "0";
        server = fact->createServer(d);

        // Revoked certificate is accpeted because IceSSL.RevocationCheck=0 disable revocation checks
        server->ice_ping();
        info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
        test(info->errorCode == IceSSL::TrustError::NoError);
        test(info->verified);
        fact->destroyServer(server);
        comm->destroy();

        // Repeat enabling revocation checks
        initData.properties = createClientProps(defaultProps, p12, "", "cacert3");
        initData.properties->setProperty("IceSSL.RevocationCheck", "1");
        initData.properties->setProperty("IceSSL.RevocationCheckCacheOnly", "0");
        initData.properties->setProperty("IceSSL.CertificateRevocationListFiles", "ca.crl.pem");
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);

        d = createServerProps(defaultProps, p12, "s_rsa_ca3_revoked", "");
        d["IceSSL.VerifyPeer"] = "0";
        server = fact->createServer(d);

        server->ice_ping();
        info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
        test(!info->verified);
        test(info->errorCode == IceSSL::TrustError::Revoked);

        fact->destroyServer(server);
        comm->destroy();

        // Test with s_rsa_cai3 only the intermediate CA cert is revoked
        const char* certificates[] = {"/s_rsa_cai3.p12", 0};
        ImportCerts import(defaultDir, certificates);

        initData.properties = createClientProps(defaultProps, p12, "", "cacert3");
        initData.properties->setProperty("IceSSL.RevocationCheck", "2");
        initData.properties->setProperty("IceSSL.RevocationCheckCacheOnly", "0");
        // CLR file used by OpenSSL, OpenSSL doesn't check the CRL distribution points.
        initData.properties->setProperty("IceSSL.CertificateRevocationListFiles", "ca.crl.pem");
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);

        d = createServerProps(defaultProps, p12, "s_rsa_cai3", "");
        d["IceSSL.VerifyPeer"] = "0";
        server = fact->createServer(d);

        server->ice_ping();
        info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
        test(!info->verified);
        test(info->errorCode == IceSSL::TrustError::Revoked);

        fact->destroyServer(server);
        comm->destroy();

        // Repeat checking only the end cert
        initData.properties = createClientProps(defaultProps, p12, "", "cacert3");
        initData.properties->setProperty("IceSSL.RevocationCheck", "1");
        initData.properties->setProperty("IceSSL.RevocationCheckCacheOnly", "0");
        // CLR file used by OpenSSL, OpenSSL doesn't check the CRL distribution points.
        initData.properties->setProperty("IceSSL.CertificateRevocationListFiles", "ca.crl.pem");
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");

        comm = initialize(initData);
        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);

        d = createServerProps(defaultProps, p12, "s_rsa_cai3", "");
        d["IceSSL.VerifyPeer"] = "0";
        server = fact->createServer(d);

        server->ice_ping();
        info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
        test(info->verified);
        test(info->errorCode == IceSSL::TrustError::NoError);

        fact->destroyServer(server);
        comm->destroy();
        import.cleanup();

        cout << "ok" << endl;
#endif
    }

    {
#if defined(ICE_USE_SCHANNEL) || defined(ICE_USE_SECURE_TRANSPORT_MACOS)
        cout << "testing certificate revocation using OCSP... " << flush;
        CommunicatorPtr comm;
        InitializationData initData;

        // First test with non revoked certificate that include AIA info
        initData.properties = createClientProps(defaultProps, p12, "", "cacert4");
        initData.properties->setProperty("IceSSL.RevocationCheck", "1");
        initData.properties->setProperty("IceSSL.RevocationCheckCacheOnly", "0");
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");
        comm = initialize(initData);
        optional<Test::ServerFactoryPrx> fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);

        Test::Properties d = createServerProps(defaultProps, p12, "s_rsa_ca4", "");
        d["IceSSL.VerifyPeer"] = "0";
        optional<Test::ServerPrx> server = fact->createServer(d);

        server->ice_ping();
        info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
        test(info->errorCode == IceSSL::TrustError::NoError);
        test(info->verified);

        fact->destroyServer(server);
        comm->destroy();

        // Now check with a revoked certificate and RevocationCheck=0 to disable revocation checks
#    ifndef ICE_USE_SECURE_TRANSPORT
        // With secure transport there is no realiable way to disable revocation checks
        initData.properties = createClientProps(defaultProps, p12, "", "cacert4");
        initData.properties->setProperty("IceSSL.RevocationCheck", "0");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);

        d = createServerProps(defaultProps, p12, "s_rsa_ca4_revoked", "");
        d["IceSSL.VerifyPeer"] = "0";
        server = fact->createServer(d);
        server->ice_ping();
        fact->destroyServer(server);
        comm->destroy();
#    endif

        // Repeat with RevoactionCheck=2 to check whole chain
        initData.properties = createClientProps(defaultProps, p12, "", "cacert4");
        initData.properties->setProperty("IceSSL.RevocationCheck", "2");
        initData.properties->setProperty("IceSSL.RevocationCheckCacheOnly", "0");
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);

        d = createServerProps(defaultProps, p12, "s_rsa_ca4", "");
        d["IceSSL.VerifyPeer"] = "0";
        server = fact->createServer(d);

        server->ice_ping();
        info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
        test(info->errorCode == IceSSL::TrustError::NoError);
        test(info->verified);
        fact->destroyServer(server);
        comm->destroy();

        // Test with s_rsa_cai4 only the intermediate CA cert is revoked
        const char* certificates[] = {"/s_rsa_cai4.p12", 0};
        ImportCerts import(defaultDir, certificates);
        initData.properties = createClientProps(defaultProps, p12, "", "cacert4");
        initData.properties->setProperty("IceSSL.RevocationCheck", "2");
        initData.properties->setProperty("IceSSL.RevocationCheckCacheOnly", "0");
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);

        d = createServerProps(defaultProps, p12, "s_rsa_cai4", "");
        d["IceSSL.VerifyPeer"] = "0";
        server = fact->createServer(d);

        server->ice_ping();
        info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
        test(!info->verified);
        test(info->errorCode == IceSSL::TrustError::Revoked);
        fact->destroyServer(server);
        comm->destroy();

        // Repeat with RevocationCheck=1 to only check the end cert
#    ifndef ICE_USE_SECURE_TRANSPORT
        // SecureTransport always check the whole chain for revocation
        initData.properties = createClientProps(defaultProps, p12, "", "cacert4");
        initData.properties->setProperty("IceSSL.RevocationCheck", "1");
        initData.properties->setProperty("IceSSL.RevocationCheckCacheOnly", "0");
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");

        comm = initialize(initData);
        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);

        d = createServerProps(defaultProps, p12, "s_rsa_cai4", "");
        d["IceSSL.VerifyPeer"] = "0";
        server = fact->createServer(d);

        server->ice_ping();
        info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
        test(info->verified);
        test(info->errorCode == IceSSL::TrustError::NoError);
        fact->destroyServer(server);
        comm->destroy();
#    endif

        // Repeat with a certificate that is unknow for the OCSP responder
        initData.properties = createClientProps(defaultProps, p12, "", "cacert4");
        initData.properties->setProperty("IceSSL.RevocationCheck", "1");
        initData.properties->setProperty("IceSSL.RevocationCheckCacheOnly", "0");
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");

        comm = initialize(initData);
        fact = Test::ServerFactoryPrx(comm, factoryRef);
        test(fact);

        d = createServerProps(defaultProps, p12, "s_rsa_ca4_unknown", "");
        d["IceSSL.VerifyPeer"] = "0";
        server = fact->createServer(d);

        server->ice_ping();
        info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(server->ice_getConnection()->getInfo());
        test(!info->verified);
        test(info->errorCode == IceSSL::TrustError::RevocationStatusUnknown);
        fact->destroyServer(server);
        comm->destroy();

        import.cleanup();

        cout << "ok" << endl;
#endif
    }

#if !defined(_AIX) && !(defined(_WIN32) && defined(ICE_USE_OPENSSL))
    //
    // On AIX 6.1, the default root certificates don't validate demo.zeroc.com.
    // On Windows with OpenSSL there aren't any system CAs.
    //
    cout << "testing system CAs... " << flush;
    {
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
            initData.properties->setProperty("Ice.Override.Timeout", "5000"); // 5s timeout
            initData.properties->setProperty("IceSSL.CheckCertName", "2");
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
                        (dynamic_cast<const Ice::SocketException*>(&ex)) ||
                        (dynamic_cast<const Ice::DNSException*>(&ex)))
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
            initData.properties->setProperty("Ice.Override.Timeout", "5000"); // 5s timeout
            initData.properties->setProperty("IceSSL.UsePlatformCAs", "1");
            initData.properties->setProperty("IceSSL.CheckCertName", "2");
            CommunicatorPtr comm = initialize(initData);
            Ice::ObjectPrx p(comm, "Glacier2/router:wss -p 443 -h zeroc.com -r /demo-proxy/chat/glacier2");
            while (true)
            {
                try
                {
                    Ice::WSConnectionInfoPtr wsinfo =
                        dynamic_pointer_cast<Ice::WSConnectionInfo>(p->ice_getConnection()->getInfo());
                    IceSSL::ConnectionInfoPtr sslInfo =
                        dynamic_pointer_cast<IceSSL::ConnectionInfo>(wsinfo->underlying);
                    test(sslInfo->verified);
                    test(sslInfo->host == "zeroc.com");
                    break;
                }
                catch (const Ice::LocalException& ex)
                {
                    if ((dynamic_cast<const Ice::ConnectTimeoutException*>(&ex)) ||
                        (dynamic_cast<const Ice::SocketException*>(&ex)) ||
                        (dynamic_cast<const Ice::DNSException*>(&ex)))
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
    }
    cout << "ok" << endl;
#endif

    return factory;
}
