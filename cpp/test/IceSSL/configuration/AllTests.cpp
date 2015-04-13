// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceSSL/Plugin.h>
#include <TestCommon.h>
#include <Test.h>
#include <fstream>

using namespace std;
using namespace Ice;


void
readFile(const string& file, vector<char>& buffer)
{
    ifstream is(file.c_str(), ios::in | ios::binary);
    if(!is.good())
    {
        throw "error opening file " + file;
    }

    is.seekg(0, is.end);
    buffer.resize(static_cast<int>(is.tellg()));
    is.seekg(0, is.beg);

    is.read(&buffer[0], buffer.size());

    if(!is.good())
    {
        throw "error reading file " + file;
    }
}

#ifdef ICE_USE_SCHANNEL
void
findCertsCleanup(HCERTSTORE store, const vector<HCERTSTORE>& stores, const vector<PCCERT_CONTEXT>& certs)
{
    for(vector<PCCERT_CONTEXT>::const_iterator i = certs.begin(); i != certs.end(); ++i)
    {
        PCCERT_CONTEXT cert = *i;


        DWORD size = 0;
        //
        // Retrieve the certificate CERT_KEY_PROV_INFO_PROP_ID property, we use the CRYPT_KEY_PROV_INFO
        // data to then remove the key set associated with the certificate.
        //
        if(CertGetCertificateContextProperty(cert, CERT_KEY_PROV_INFO_PROP_ID, 0, &size))
        {
            vector<char> buf(size);
            if(CertGetCertificateContextProperty(cert, CERT_KEY_PROV_INFO_PROP_ID, &buf[0], &size))
            {
                CRYPT_KEY_PROV_INFO* keyProvInfo = reinterpret_cast<CRYPT_KEY_PROV_INFO*>(&buf[0]);
                HCRYPTPROV cryptProv = 0;
                if(CryptAcquireContextW(&cryptProv, keyProvInfo->pwszContainerName, keyProvInfo->pwszProvName,
                                        keyProvInfo->dwProvType, 0))
                {
                    CryptAcquireContextW(&cryptProv, keyProvInfo->pwszContainerName, keyProvInfo->pwszProvName,
                                         keyProvInfo->dwProvType, CRYPT_DELETEKEYSET);
                }
            }
            CertDeleteCertificateFromStore(cert);
        }


    }
    for(vector<HCERTSTORE>::const_iterator i = stores.begin(); i != stores.end(); ++i)
    {
        CertCloseStore(*i, 0);
    }
}
#endif

class PasswordPromptI : public IceSSL::PasswordPrompt
{
public:

    PasswordPromptI(const string& password) : _password(password), _count(0)
    {
    }

    virtual string getPassword()
    {
        ++_count;
        return _password;
    }

    int count() const
    {
        return _count;
    }

private:

    string _password;
    int _count;
};
typedef IceUtil::Handle<PasswordPromptI> PasswordPromptIPtr;

class CertificateVerifierI : public IceSSL::CertificateVerifier
{
public:

    CertificateVerifierI()
    {
        reset();
    }

    virtual bool
    verify(const IceSSL::NativeConnectionInfoPtr& info)
    {
        if(info->nativeCerts.size() > 0)
        {
            //
            // Subject alternative name
            //
            {
                vector<pair<int, string> > altNames = info->nativeCerts[0]->getSubjectAlternativeNames();
                vector<string> ipAddresses;
                vector<string> dnsNames;
                for(vector<pair<int, string> >::const_iterator p = altNames.begin(); p != altNames.end(); ++p)
                {
                    if(p->first == 7)
                    {
                        ipAddresses.push_back(p->second);
                    }
                    else if(p->first == 2)
                    {
                        dnsNames.push_back(p->second);
                    }

                }

                test(find(dnsNames.begin(), dnsNames.end(), "server") != dnsNames.end());
                test(find(ipAddresses.begin(), ipAddresses.end(), "127.0.0.1") != ipAddresses.end());
            }

            //
            // Issuer alternative name
            //
            {
                vector<pair<int, string> > altNames = info->nativeCerts[0]->getIssuerAlternativeNames();
                vector<string> ipAddresses;
                vector<string> emailAddresses;
                for(vector<pair<int, string> >::const_iterator p = altNames.begin(); p != altNames.end(); ++p)
                {
                    if(p->first == 7)
                    {
                        ipAddresses.push_back(p->second);
                    }
                    else if(p->first == 1)
                    {
                        emailAddresses.push_back(p->second);
                    }
                }

                test(find(ipAddresses.begin(), ipAddresses.end(), "127.0.0.1") != ipAddresses.end());
                test(find(emailAddresses.begin(), emailAddresses.end(), "issuer@zeroc.com") != emailAddresses.end());
            }
        }

        _hadCert = info->nativeCerts.size() != 0;
        _invoked = true;
        return _returnValue;
    }

    void reset()
    {
        _returnValue = true;
        _invoked = false;
        _hadCert = false;
    }

    void returnValue(bool b)
    {
        _returnValue = b;
    }

    bool invoked() const
    {
        return _invoked;
    }

    bool hadCert() const
    {
        return _hadCert;
    }

private:

    bool _returnValue;
    bool _invoked;
    bool _hadCert;
};
typedef IceUtil::Handle<CertificateVerifierI> CertificateVerifierIPtr;

int keychainN = 0;

static PropertiesPtr
createClientProps(const Ice::PropertiesPtr& defaultProperties, const string& defaultDir,
                  const string& defaultHost, bool password)
{
    PropertiesPtr result = createProperties();
    result->setProperty("Ice.Plugin.IceSSL", "IceSSL:createIceSSL");
    result->setProperty("IceSSL.DefaultDir", defaultDir);
    if(!defaultProperties->getProperty("Ice.IPv6").empty())
    {
        result->setProperty("Ice.IPv6", defaultProperties->getProperty("Ice.IPv6"));
    }
    if(!defaultHost.empty())
    {
        result->setProperty("Ice.Default.Host", defaultHost);
    }
    if(password)
    {
        result->setProperty("IceSSL.Password", "password");
    }
    //result->setProperty("IceSSL.Trace.Security", "1");
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
createServerProps(const Ice::PropertiesPtr& defaultProperties, const string& defaultDir, const string& defaultHost,
                  bool password)
{
    Test::Properties result;
    result["Ice.Plugin.IceSSL"] = "IceSSL:createIceSSL";
    result["IceSSL.DefaultDir"] = defaultDir;
    if(!defaultProperties->getProperty("Ice.IPv6").empty())
    {
        result["Ice.IPv6"] = defaultProperties->getProperty("Ice.IPv6");
    }
    if(!defaultHost.empty())
    {
        result["Ice.Default.Host"] = defaultHost;
    }
    if(password)
    {
        result["IceSSL.Password"] = "password";
    }
    //result["IceSSL.Trace.Security"] = "1";
#ifdef ICE_USE_SECURE_TRANSPORT
    ostringstream keychainName;
    keychainName << "../certs/keychain//server" << keychainN << ".keychain";
    result["IceSSL.Keychain"] = keychainName.str();
    result["IceSSL.KeychainPassword"] = "password";
#endif
    return result;
}

void verify(const IceSSL::CertificatePtr& cert, const IceSSL::CertificatePtr& ca)
{
    cerr << "Verify signature: ";
    if(cert->verify(ca))
    {
        cerr << " VALID";
    }
    else
    {
        cerr << " INVALID";
    }
    cerr << endl;
}

void
allTests(const CommunicatorPtr& communicator, const string& testDir, bool pfx, bool shutdown)
{
    string factoryRef = "factory:tcp -p 12010";
    ObjectPrx base = communicator->stringToProxy(factoryRef);
    test(base);
    Test::ServerFactoryPrx factory = Test::ServerFactoryPrx::checkedCast(base);

    string defaultHost = communicator->getProperties()->getProperty("Ice.Default.Host");
    string defaultDir = testDir + "/../certs";
    Ice::PropertiesPtr defaultProperties = communicator->getProperties();
#ifdef _WIN32
    string sep = ";";
#else
    string sep = ":";
#endif

    cout << "testing manual initialization... " << flush;
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("Ice.InitPlugins", "0");
        CommunicatorPtr comm = initialize(initData);
        ObjectPrx p = comm->stringToProxy("dummy:ssl -p 9999");
        try
        {
            p->ice_ping();
            test(false);
        }
        catch(const PluginInitializationException&)
        {
            // Expected.
        }
        catch(const LocalException&)
        {
            test(false);
        }
        comm->destroy();
    }

//
// Anonymous cipher are not supported with SChannel
//
#ifndef ICE_USE_SCHANNEL
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("Ice.InitPlugins", "0");
#  ifdef ICE_USE_OPENSSL
        initData.properties->setProperty("IceSSL.Ciphers", "ADH");
#  else
        initData.properties->setProperty("IceSSL.Ciphers", "DH_anon_WITH_AES_256_CBC_SHA");
#  endif
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");
        CommunicatorPtr comm = initialize(initData);
        PluginManagerPtr pm = comm->getPluginManager();
        pm->initializePlugins();
        ObjectPrx obj = comm->stringToProxy(factoryRef);
        test(obj);
        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(obj);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
#  ifdef ICE_USE_OPENSSL
        d["IceSSL.Ciphers"] = "ADH";
#  else
        d["IceSSL.Ciphers"] = "DH_anon_WITH_AES_256_CBC_SHA";
#  endif
        d["IceSSL.VerifyPeer"] = "0";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
#endif
    cout << "ok" << endl;

    cout << "testing certificate verification... " << flush;
    {
        //
        // Test IceSSL.VerifyPeer=0. Client does not have a certificate,
        // but it still verifies the server's. The createClientProps
        // function defines IceSSL.DefaultDir, which allows OpenSSL in the
        // client to find the CA certificate. We remove that property here
        // to verify that the connection can still proceed without any
        // CA certificate.
        //
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");
        initData.properties->setProperty("IceSSL.DefaultDir", "");
        CommunicatorPtr comm = initialize(initData);
        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);

        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";

        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.VerifyPeer"] = "0";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->noCert();
        }
        catch(const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        //
        // Validate that we can get the connection info.
        //
        try
        {
            IceSSL::NativeConnectionInfoPtr info =
                IceSSL::NativeConnectionInfoPtr::dynamicCast(server->ice_getConnection()->getInfo());
#if defined(ICE_USE_SCHANNEL) || defined(ICE_USE_SECURE_TRANSPORT)
            //
            // SChannel doesn't seem to send the root certificate
            //
            test(info->nativeCerts.size() == 1);
#else
            test(info->nativeCerts.size() == 2);
#endif
        }
        catch(const Ice::LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);

        //
        // Test IceSSL.VerifyPeer=1. Client does not have a certificate.
        //
        d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.VerifyPeer"] = "1";
        server = fact->createServer(d);
        try
        {
            server->noCert();
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);

        //
        // Test IceSSL.VerifyPeer=2. This should fail because the client
        // does not supply a certificate.
        //
        d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.VerifyPeer"] = "2";
        server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const ProtocolException&)
        {
            // Expected.
        }
#if defined(_WIN32) || defined(ICE_USE_SECURE_TRANSPORT)
        catch(const ConnectionLostException&)
        {
            // Expected.
        }
#endif
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);

        comm->destroy();

        //
        // Test IceSSL.VerifyPeer=1. Client has a certificate.
        //
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }

        initData.properties->setProperty("IceSSL.VerifyPeer", "0");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";

        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.VerifyPeer"] = "1";
        server = fact->createServer(d);

        try
        {
            IceSSL::CertificatePtr clientCert = IceSSL::Certificate::load(defaultDir + "/c_rsa_nopass_ca1_pub.pem");
            server->checkCert(clientCert->getSubjectDN(), clientCert->getIssuerDN());

            //
            // Validate that we can get the connection info. Validate
            // that the certificates have the same DN.
            //
            // Validate some aspects of the Certificate class.
            //
            IceSSL::CertificatePtr serverCert = IceSSL::Certificate::load(defaultDir + "/s_rsa_nopass_ca1_pub.pem");
            test(IceSSL::Certificate::decode(serverCert->encode()) == serverCert);
            test(serverCert == serverCert);
            test(serverCert->checkValidity());
            test(!serverCert->checkValidity(IceUtil::Time::seconds(0)));

            IceSSL::CertificatePtr caCert = IceSSL::Certificate::load(defaultDir + "/cacert1.pem");
            test(caCert == caCert);
            test(caCert->checkValidity());
            test(!caCert->checkValidity(IceUtil::Time::seconds(0)));

            test(!serverCert->verify(serverCert));
            test(serverCert->verify(caCert));
            test(caCert->verify(caCert));

            IceSSL::NativeConnectionInfoPtr info =
                IceSSL::NativeConnectionInfoPtr::dynamicCast(server->ice_getConnection()->getInfo());

            test(info->nativeCerts.size() == 2);

            test(caCert == info->nativeCerts[1]);
            test(serverCert == info->nativeCerts[0]);

            test(serverCert != info->nativeCerts[1]);
            test(caCert != info->nativeCerts[0]);

            test(info->nativeCerts[0]->checkValidity() && info->nativeCerts[1]->checkValidity());
            test(!info->nativeCerts[0]->checkValidity(IceUtil::Time::seconds(0)) &&
                 !info->nativeCerts[1]->checkValidity(IceUtil::Time::seconds(0)));
            test(info->nativeCerts[0]->verify(info->nativeCerts[1]));
            test(info->nativeCerts.size() == 2 &&
                 info->nativeCerts[0]->getSubjectDN() == serverCert->getSubjectDN() &&
                 info->nativeCerts[0]->getIssuerDN() == serverCert->getIssuerDN());
        }
        catch(const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);

        //
        // Test IceSSL.VerifyPeer=2. Client has a certificate.
        //
        d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.VerifyPeer"] = "2";
        server = fact->createServer(d);
        try
        {
            IceSSL::CertificatePtr clientCert = IceSSL::Certificate::load(defaultDir + "/c_rsa_nopass_ca1_pub.pem");
            server->checkCert(clientCert->getSubjectDN(), clientCert->getIssuerDN());
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);

        comm->destroy();

        //
        // Test IceSSL.VerifyPeer=1. This should fail because the client doesn't
        // trust the server's CA. We disable IceSSL.DefaultDir in the client so that
        // OpenSSL can't search for the server's CA certificate.
        //
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.DefaultDir", "");
        initData.properties->setProperty("IceSSL.VerifyPeer", "1");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.VerifyPeer"] = "0";
        server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const ProtocolException&)
        {
            // Expected.
        }
#if defined(_WIN32) || defined(ICE_USE_SECURE_TRANSPORT)
        catch(const ConnectionLostException&)
        {
            // Expected.
        }
#endif
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);

        comm->destroy();

        //
        // Test IceSSL.VerifyPeer=1. This should fail because the server doesn't
        // trust the client's CA. The IceSSL.DefaultDir setting in the client
        // allows OpenSSL to find the server's CA certificate. We have to disable
        // IceSSL.DefaultDir in the server so that it can't find the client's CA
        // certificate.
        //
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert2.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca2.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca2_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca2_priv.pem");
        }
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d.erase("IceSSL.DefaultDir");
        d["IceSSL.CertAuthFile"] = defaultDir + "/cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = defaultDir + "/s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = defaultDir + "/s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = defaultDir + "/s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.VerifyPeer"] = "1";
        server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const ProtocolException&)
        {
            // Expected.
        }
#if defined(_WIN32) || defined(ICE_USE_SECURE_TRANSPORT)
        catch(const ConnectionLostException&)
        {
            // Expected.
        }
#endif
        catch(const LocalException&)
        {
           test(false);
        }
        fact->destroyServer(server);

        comm->destroy();

        //
        // This should succeed because the self signed certificate used by the server is
        // trusted.
        //
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert2.pem");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        if(pfx)
        {
            d["IceSSL.CertFile"] = defaultDir + "/cacert2.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = defaultDir + "/cacert2.pem";
            d["IceSSL.KeyFile"] = defaultDir + "/cakey2.pem";
        }
        d["IceSSL.VerifyPeer"] = "0";
        server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);

        comm->destroy();

        //
        // This should fail because the self signed certificate used by the server is not
        // trusted.  The IceSSL.DefaultDir setting in the client allows OpenSSL to find
        // the server's CA certificate. We have to disable IceSSL.DefaultDir in the client
        // so that it can't find the server's CA certificate.
        //
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.DefaultDir", "");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        if(pfx)
        {
            d["IceSSL.CertFile"] = defaultDir + "/cacert2.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = defaultDir + "/cacert2.pem";
            d["IceSSL.KeyFile"] = defaultDir + "/cakey2.pem";
        }
        d["IceSSL.VerifyPeer"] = "0";
        server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const ProtocolException&)
        {
            // Expected.
        }
#ifdef _WIN32
        catch(const ConnectionLostException&)
        {
            // Expected.
        }
#endif
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);

        comm->destroy();

        //
        // Verify that IceSSL.CheckCertName has no effect in a server.
        //
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        comm = initialize(initData);

        fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.CheckCertName"] = "1";
        server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();

        //
        // Test IceSSL.CheckCertName. The test certificates for the server contain "127.0.0.1"
        // as the common name or as a subject alternative name, so we only perform this test when
        // the default host is "127.0.0.1".
        //
        if(defaultHost == "127.0.0.1")
        {
            //
            // Test subject alternative name.
            //
            {
                initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
                initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
                if(pfx)
                {
                    initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
                }
                else
                {
                    initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
                    initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
                }
                initData.properties->setProperty("IceSSL.CheckCertName", "1");
                comm = initialize(initData);

                fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
                test(fact);
                d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
                d["IceSSL.CertAuthFile"] = "cacert1.pem";
                if(pfx)
                {
                    d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
                }
                else
                {
                    d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
                    d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
                }
                server = fact->createServer(d);
                try
                {
                    server->ice_ping();
                }
                catch(const LocalException&)
                {
                    test(false);
                }
                fact->destroyServer(server);
                comm->destroy();
            }
            //
            // Test common name.
            //
            {
                initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
                initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
                if(pfx)
                {
                    initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
                }
                else
                {
                    initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
                    initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
                }
                initData.properties->setProperty("IceSSL.CheckCertName", "1");
                comm = initialize(initData);

                fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
                test(fact);
                d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
                d["IceSSL.CertAuthFile"] = "cacert1.pem";
                if(pfx)
                {
                    d["IceSSL.CertFile"] = "s_rsa_ca1_cn1.pfx";
                }
                else
                {
                    d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_cn1_pub.pem";
                    d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_cn1_priv.pem";
                }
                server = fact->createServer(d);
                try
                {
                    server->ice_ping();
                }
                catch(const LocalException&)
                {
                    test(false);
                }
                fact->destroyServer(server);
                comm->destroy();
            }
            //
            // Test common name again. The certificate used in this test has "127.0.0.11" as its
            // common name, therefore the address "127.0.0.1" must NOT match.
            //
            {
                initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
                initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
                if(pfx)
                {
                    initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
                }
                else
                {
                    initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
                    initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
                }
                initData.properties->setProperty("IceSSL.CheckCertName", "1");
                comm = initialize(initData);

                fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
                test(fact);
                d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
                d["IceSSL.CertAuthFile"] = "cacert1.pem";
                if(pfx)
                {
                    d["IceSSL.CertFile"] = "s_rsa_ca1_cn2.pfx";
                }
                else
                {
                    d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_cn2_pub.pem";
                    d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_cn2_priv.pem";
                }
                server = fact->createServer(d);
                try
                {
                    server->ice_ping();
                    test(false);
                }
                catch(const LocalException&)
                {
                    // Expected.
                }
                fact->destroyServer(server);
                comm->destroy();
            }
        }
    }
    cout << "ok" << endl;

    cout << "testing custom certificate verifier... " << flush;

    {
//
// Anonymous ciphers are not supported with SChannel.
//
#ifndef ICE_USE_SCHANNEL
        //
        // ADH is allowed but will not have a certificate.
        //
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
#  ifdef ICE_USE_OPENSSL
        initData.properties->setProperty("IceSSL.Ciphers", "ADH");
#  else
        initData.properties->setProperty("IceSSL.Ciphers", "(DH_anon*)");
#  endif
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");
        CommunicatorPtr comm = initialize(initData);
        IceSSL::PluginPtr plugin = IceSSL::PluginPtr::dynamicCast(comm->getPluginManager()->getPlugin("IceSSL"));
        test(plugin);
        CertificateVerifierIPtr verifier = new CertificateVerifierI;
        plugin->setCertificateVerifier(verifier);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
#  ifdef ICE_USE_OPENSSL
        string cipherSub = "ADH-";
        d["IceSSL.Ciphers"] = "ADH";
#  else
        string cipherSub = "DH_anon";
        d["IceSSL.Ciphers"] = "(DH_anon*)";
#  endif
        d["IceSSL.VerifyPeer"] = "0";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->checkCipher(cipherSub);
            IceSSL::NativeConnectionInfoPtr info =
                IceSSL::NativeConnectionInfoPtr::dynamicCast(server->ice_getConnection()->getInfo());
            test(info->cipher.compare(0, cipherSub.size(), cipherSub) == 0);
        }
        catch(const LocalException&)
        {
            test(false);
        }
        test(verifier->invoked());
        test(!verifier->hadCert());

        //
        // Have the verifier return false. Close the connection explicitly
        // to force a new connection to be established.
        //
        verifier->reset();
        verifier->returnValue(false);
        server->ice_getConnection()->close(false);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const SecurityException&)
        {
            // Expected.
        }
        catch(const LocalException&)
        {
            test(false);
        }
        test(verifier->invoked());
        test(!verifier->hadCert());

        fact->destroyServer(server);
        comm->destroy();
#endif
    }
    {
        //
        // Verify that a server certificate is present.
        //
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");
        CommunicatorPtr comm = initialize(initData);
        IceSSL::PluginPtr plugin = IceSSL::PluginPtr::dynamicCast(comm->getPluginManager()->getPlugin("IceSSL"));
        test(plugin);
        CertificateVerifierIPtr verifier = new CertificateVerifierI;
        plugin->setCertificateVerifier(verifier);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.VerifyPeer"] = "2";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException&)
        {
            test(false);
        }
        test(verifier->invoked());
        test(verifier->hadCert());
        fact->destroyServer(server);
        comm->destroy();
    }
    cout << "ok" << endl;


    cout << "testing protocols... " << flush;
    {
        //
        // In OS X we don't support IceSSL.Protocols as secure transport doesn't allow to set the enabled protocols
        // instead we use IceSSL.ProtocolVersionMax IceSSL.ProtocolVersionMin to set the maximun and minimum
        // enabled protocol versions. See the test bellow.
        //
#ifndef ICE_USE_SECURE_TRANSPORT
        //
        // This should fail because the client and server have no protocol
        // in common.
        //
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");
        initData.properties->setProperty("IceSSL.Protocols", "ssl3");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.VerifyPeer"] = "0";
        d["IceSSL.Protocols"] = "tls";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const ProtocolException&)
        {
            // Expected on some platforms.
        }
        catch(const ConnectionLostException&)
        {
            // Expected on some platforms.
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();

        //
        // This should succeed.
        //
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.VerifyPeer"] = "0";
        d["IceSSL.Protocols"] = "tls, ssl3";
        server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();

        //
        // This should fail because the client only accept SSLv3 and the server
        // use the default protocol set that disables SSLv3
        //
        {
            InitializationData initData;
            initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
            initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
            if(pfx)
            {
                initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
            }
            else
            {
                initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
                initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
            }
            initData.properties->setProperty("IceSSL.VerifyPeer", "0");
            initData.properties->setProperty("IceSSL.Protocols", "ssl3");
            CommunicatorPtr comm = initialize(initData);

            Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
            test(fact);
            Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
            d["IceSSL.CertAuthFile"] = "cacert1.pem";
            if(pfx)
            {
                d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
            }
            else
            {
                d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
                d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
            }
            d["IceSSL.VerifyPeer"] = "0";
            Test::ServerPrx server = fact->createServer(d);
            try
            {
                server->ice_ping();
                test(false);
            }
            catch(const ProtocolException&)
            {
                // Expected on some platforms.
            }
            catch(const ConnectionLostException&)
            {
                // Expected on some platforms.
            }
            catch(const LocalException&)
            {
                test(false);
            }
            fact->destroyServer(server);
            comm->destroy();
        }

        //
        // This should success because both have SSLv3 enabled
        //
        {
            InitializationData initData;
            initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
            initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
            initData.properties->setProperty("IceSSL.Protocols", "ssl3");
            CommunicatorPtr comm = initialize(initData);

            Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
            test(fact);
            Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
            d["IceSSL.CertAuthFile"] = "cacert1.pem";
            if(pfx)
            {
                d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
            }
            else
            {
                d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
                d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
            }
            d["IceSSL.VerifyPeer"] = "0";
            d["IceSSL.Protocols"] = "ssl3, tls, tls1_1, tls1_2";
            Test::ServerPrx server = fact->createServer(d);
            try
            {
                server->ice_ping();
            }
            catch(const LocalException& ex)
            {
                cerr << ex << endl;
                test(false);
            }
            fact->destroyServer(server);
            comm->destroy();
        }
#else
        //
        // This should fail because the client and server have no protocol
        // in common.
        //
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.Ciphers", "(DH_anon*)");
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");
        initData.properties->setProperty("IceSSL.ProtocolVersionMax", "ssl3");
        initData.properties->setProperty("IceSSL.ProtocolVersionMin", "ssl3");
        CommunicatorPtr comm = initialize(initData);
        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.Ciphers"] = "(DH_anon*)";
        d["IceSSL.VerifyPeer"] = "0";
        d["IceSSL.ProtocolVersionMax"] = "tls1_2";
        d["IceSSL.ProtocolVersionMin"] = "tls1_2";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const ProtocolException&)
        {
            // Expected on some platforms.
        }
        catch(const ConnectionLostException&)
        {
            // Expected on some platforms.
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();

        //
        // This should succeed.
        //
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.Ciphers"] = "(DH_anon*)";
        d["IceSSL.VerifyPeer"] = "0";
        d["IceSSL.ProtocolVersionMax"] = "tls1";
        d["IceSSL.ProtocolVersionMin"] = "ssl3";
        server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();

        //
        // This should fail because the client only accept SSLv3 and the server
        // use the default protocol set that disables SSLv3
        //
        {
            InitializationData initData;
            initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
            initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
            if(pfx)
            {
                initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
            }
            else
            {
                initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
                initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
            }
            initData.properties->setProperty("IceSSL.VerifyPeer", "0");
            initData.properties->setProperty("IceSSL.ProtocolVersionMin", "ssl3");
            initData.properties->setProperty("IceSSL.ProtocolVersionMax", "ssl3");
            CommunicatorPtr comm = initialize(initData);

            Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
            test(fact);
            Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
            d["IceSSL.CertAuthFile"] = "cacert1.pem";
            if(pfx)
            {
                d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
            }
            else
            {
                d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
                d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
            }
            d["IceSSL.VerifyPeer"] = "0";
            Test::ServerPrx server = fact->createServer(d);
            try
            {
                server->ice_ping();
                test(false);
            }
            catch(const ProtocolException&)
            {
                // Expected on some platforms.
            }
            catch(const ConnectionLostException&)
            {
                // Expected on some platforms.
            }
            catch(const LocalException&)
            {
                test(false);
            }
            fact->destroyServer(server);
            comm->destroy();
        }

        //
        // This should success because both have SSLv3 enabled
        //
        {
            InitializationData initData;
            initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
            initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
            if(pfx)
            {
                initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
            }
            else
            {
                initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
                initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
            }
            initData.properties->setProperty("IceSSL.VerifyPeer", "0");
            initData.properties->setProperty("IceSSL.ProtocolVersionMin", "ssl3");
            initData.properties->setProperty("IceSSL.ProtocolVersionMax", "ssl3");
            CommunicatorPtr comm = initialize(initData);

            Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
            test(fact);
            Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
            d["IceSSL.CertAuthFile"] = "cacert1.pem";
            if(pfx)
            {
                d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
            }
            else
            {
                d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
                d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
            }
            d["IceSSL.VerifyPeer"] = "0";
            d["IceSSL.ProtocolVersionMin"] = "ssl3";
            Test::ServerPrx server = fact->createServer(d);
            try
            {
                server->ice_ping();
            }
            catch(const LocalException&)
            {
                test(false);
            }
            fact->destroyServer(server);
            comm->destroy();
        }
#endif
    }
    cout << "ok" << endl;

    cout << "testing expired certificates... " << flush;
    {
        //
        // This should fail because the server's certificate is expired.
        //
        {
            IceSSL::CertificatePtr cert = IceSSL::Certificate::load(defaultDir + "/s_rsa_nopass_ca1_exp_pub.pem");
            test(!cert->checkValidity());
        }

        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        CommunicatorPtr comm = initialize(initData);
        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1_exp.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_exp_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_exp_priv.pem";
        }
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const ProtocolException&)
        {
            // Expected.
        }
#if defined(_WIN32) || defined(ICE_USE_SECURE_TRANSPORT)
        catch(const ConnectionLostException&)
        {
            // Expected.
        }
#endif
        catch(const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();

        //
        // This should fail because the client's certificate is expired.
        //
        {
            IceSSL::CertificatePtr cert = IceSSL::Certificate::load(defaultDir + "/c_rsa_nopass_ca1_exp_pub.pem");
            test(!cert->checkValidity());
        }

        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1_exp.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_exp_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_exp_priv.pem");
        }
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const ProtocolException&)
        {
            // Expected.
        }
#if defined(_WIN32) || defined(ICE_USE_SECURE_TRANSPORT)
        catch(const ConnectionLostException&)
        {
            // Expected.
        }
#endif
        catch(const LocalException& ex)
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
        // Don't specify CertAuthFile explicitly; we let OpenSSL find the CA
        // certificate in the default directory.
        //
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        CommunicatorPtr comm = initialize(initData);
        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    cout << "ok" << endl;
#endif

    //
    // SChannel doesn't support PCKS8 certificates (PEM Password protected certificates)
    //
#ifdef ICE_USE_SCHANNEL
    if(pfx)
    {
#endif
    cout << "testing password prompt... " << flush;
    {
        //
        // Use the correct password.
        //
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, false);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_pass_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_pass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_pass_ca1_priv.pem");
        }
        initData.properties->setProperty("Ice.InitPlugins", "0");
        CommunicatorPtr comm = initialize(initData);
        PluginManagerPtr pm = comm->getPluginManager();
        IceSSL::PluginPtr plugin = IceSSL::PluginPtr::dynamicCast(pm->getPlugin("IceSSL"));
        test(plugin);
        PasswordPromptIPtr prompt = new PasswordPromptI("client");
        plugin->setPasswordPrompt(prompt);
        pm->initializePlugins();
        test(prompt->count() == 1);
        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();

        //
        // Use an incorrect password and check that retries are attempted.
        //
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, false);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_pass_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_pass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_pass_ca1_priv.pem");
        }
        initData.properties->setProperty("IceSSL.PasswordRetryMax", "4");
        initData.properties->setProperty("Ice.InitPlugins", "0");
        comm = initialize(initData);
        pm = comm->getPluginManager();
        plugin = IceSSL::PluginPtr::dynamicCast(pm->getPlugin("IceSSL"));
        test(plugin);
        prompt = new PasswordPromptI("invalid");
        plugin->setPasswordPrompt(prompt);
        try
        {
            pm->initializePlugins();
            test(false);
        }
        catch(const PluginInitializationException&)
        {
            // Expected.
        }
        catch(const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        test(prompt->count() == 4);
        comm->destroy();
    }
    cout << "ok" << endl;
#ifdef ICE_USE_SCHANNEL
    }
#endif
    cout << "testing ciphers... " << flush;
#ifndef ICE_USE_SCHANNEL
    {
        //
        // The server has a certificate but the client doesn't. They should
        // negotiate to use ADH since we explicitly enable it.
        //
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
#  ifdef ICE_USE_OPENSSL
        initData.properties->setProperty("IceSSL.Ciphers", "ADH");
#  else
        initData.properties->setProperty("IceSSL.Ciphers", "(DH_anon*)");
#  endif
        CommunicatorPtr comm = initialize(initData);
        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
        d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
#  ifdef ICE_USE_OPENSSL
        string cipherSub = "ADH-";
        d["IceSSL.Ciphers"] = "RSA:ADH";
#  else
        string cipherSub = "DH_";
        d["IceSSL.Ciphers"] = "(RSA_*) (DH_anon*)";
#  endif
        d["IceSSL.VerifyPeer"] = "1";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->checkCipher(cipherSub);
            IceSSL::NativeConnectionInfoPtr info =
                IceSSL::NativeConnectionInfoPtr::dynamicCast(server->ice_getConnection()->getInfo());
            test(info->cipher.compare(0, cipherSub.size(), cipherSub) == 0);
        }
        catch(const LocalException& ex)
        {
//
// OS X 10.10 bug the handshake fails attempting client auth
// with anon cipher.
//
#  ifndef ICE_USE_SECURE_TRANSPORT
            cerr << ex << endl;
            test(false);
#  endif
        }
        fact->destroyServer(server);
        comm->destroy();
    }

    {
        //
        // This should fail because we disabled all anonymous ciphers and the server doesn't
        // provide a certificate.
        //
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
#  ifdef ICE_USE_OPENSSL
        initData.properties->setProperty("IceSSL.Ciphers", "ALL:!ADH");
#  else
        initData.properties->setProperty("IceSSL.Ciphers", "ALL !(DH_anon*)");
#  endif
        CommunicatorPtr comm = initialize(initData);
        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.VerifyPeer"] = "0";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const ProtocolException&)
        {

        }
#  if defined(_WIN32) || defined(ICE_USE_SECURE_TRANSPORT)
        catch(const ConnectionLostException&)
        {

        }
#  endif
        catch(const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }

#  ifdef ICE_USE_SECURE_TRANSPORT
    {
        //
        // This should fail because the client disabled all ciphers.
        //
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
        initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        initData.properties->setProperty("IceSSL.Ciphers", "NONE");
        try
        {
            CommunicatorPtr comm = initialize(initData);
            test(false);
        }
        catch(const Ice::PluginInitializationException&)
        {
            //Expected when disabled all cipher suites.
        }
        catch(const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
    }
    {
        //
        // Test IceSSL.DHParams
        //
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.Ciphers", "(DH_anon*)");
        CommunicatorPtr comm = initialize(initData);
        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.Ciphers"] = "(DH_anon*)";
        d["IceSSL.DHParams"] = "dh_params512.der";
        d["IceSSL.VerifyPeer"] = "0";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->checkCipher("DH_anon");
        }
        catch(const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }

    {
        //
        // Test IceSSL.DHParams
        //
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.Ciphers", "(DH_anon*)");
        CommunicatorPtr comm = initialize(initData);
        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.Ciphers"] = "(DH_anon*)";
        d["IceSSL.DHParams"] = "dh_params1024.der";
        d["IceSSL.VerifyPeer"] = "0";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->checkCipher("DH_anon");
        }
        catch(const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
#  endif
#else // SChannel ciphers
    {
        //
        // Client and server should negotiate to use 3DES as it is enabled in both.
        //
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        initData.properties->setProperty("IceSSL.Ciphers", "3DES");

        CommunicatorPtr comm = initialize(initData);
        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);

        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.Ciphers"] = "3DES AES_256";

        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->checkCipher("3DES");
            IceSSL::NativeConnectionInfoPtr info =
                IceSSL::NativeConnectionInfoPtr::dynamicCast(server->ice_getConnection()->getInfo());
            test(info->cipher.compare(0, 4, "3DES") == 0);
        }
        catch(const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        //
        // Client and server doesn't enable a common cipher negotiate to use 3DES as it is enabled in both.
        //
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        initData.properties->setProperty("IceSSL.Ciphers", "3DES");

        CommunicatorPtr comm = initialize(initData);
        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);

        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.Ciphers"] = "AES_256";

        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->checkCipher("3DES");
            test(false);
        }
        catch(const Ice::ConnectionLostException&)
        {
            //expected
        }
        catch(const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
#endif
//
// No DSA support in Secure Transport.
//
#ifndef ICE_USE_SECURE_TRANSPORT
    {

    //
    // DSA PEM certificates are not supported with SChannel.
    //
#   ifdef ICE_USE_SCHANNEL
    if(pfx)
    {
#   endif
        //
        // Configure a server with RSA and DSA certificates.
        //
        // First try a client with a DSA certificate.
        //
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_dsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_dsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_dsa_nopass_ca1_priv.pem");
        }
        initData.properties->setProperty("IceSSL.Ciphers", "DEFAULT:DSS");
        CommunicatorPtr comm = initialize(initData);
        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx" + sep + "s_dsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem" + sep + "s_dsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem" + sep + "s_dsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.Ciphers"] = "DEFAULT:DSS";
        d["IceSSL.VerifyPeer"] = "1";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();

        //
        // Next try a client with an RSA certificate.
        //
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx" + sep + "s_dsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem" + sep + "s_dsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem" + sep + "s_dsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.Ciphers"] = "DEFAULT:DSS";
        d["IceSSL.VerifyPeer"] = "1";
        server = fact->createServer(d);

        try
        {
            server->ice_ping();
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
#   ifdef ICE_USE_SCHANNEL
    }
#   endif

#   ifndef ICE_USE_SCHANNEL
        //
        // Next try a client with ADH. This should fail.
        //
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.Ciphers", "ADH");
        comm = initialize(initData);
        fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem" + sep + "s_dsa_nopass_ca1_pub.pem";
        d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem" + sep + "s_dsa_nopass_ca1_priv.pem";
        d["IceSSL.Ciphers"] = "DEFAULT:DSS";
        d["IceSSL.VerifyPeer"] = "1";
        server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const ProtocolException&)
        {
            // Expected.
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
#   endif
    }
#   ifndef ICE_USE_SCHANNEL
    {
        //
        // Configure a server with RSA and a client with DSA. This should fail.
        //
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_dsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_dsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_dsa_nopass_ca1_priv.pem");
        }

        initData.properties->setProperty("IceSSL.Ciphers", "DSS");

        CommunicatorPtr comm = initialize(initData);
        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.VerifyPeer"] = "2";

        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const ProtocolException&)
        {
            // Expected.
        }
#   ifdef _WIN32
        catch(const ConnectionLostException&)
        {
            // Expected.
        }
#   endif
        catch(const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
#   endif
#endif
    cout << "ok" << endl;

    cout << "testing IceSSL.TrustOnly... " << flush;
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        initData.properties->setProperty("IceSSL.TrustOnly", "C=US, ST=Florida, O=ZeroC\\, Inc.,"
                                         "OU=Ice, emailAddress=info@zeroc.com, CN=Server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        initData.properties->setProperty("IceSSL.TrustOnly", "!C=US, ST=Florida, O=ZeroC\\, Inc.,"
                                         "OU=Ice, emailAddress=info@zeroc.com, CN=Server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        initData.properties->setProperty("IceSSL.TrustOnly", "C=US, ST=Florida, O=\"ZeroC, Inc.\","
                                         "OU=Ice, emailAddress=info@zeroc.com, CN=Server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.TrustOnly"] = "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com,CN=Client";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.TrustOnly"] = "!C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Client";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        initData.properties->setProperty("IceSSL.TrustOnly", "CN=Server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        initData.properties->setProperty("IceSSL.TrustOnly", "!CN=Server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.TrustOnly"] = "CN=Client";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.TrustOnly"] = "!CN=Client";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        initData.properties->setProperty("IceSSL.TrustOnly", "CN=Client");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.TrustOnly"] = "CN=Server";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        initData.properties->setProperty("IceSSL.TrustOnly", "C=Canada,CN=Server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        initData.properties->setProperty("IceSSL.TrustOnly", "!C=Canada,CN=Server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        initData.properties->setProperty("IceSSL.TrustOnly", "C=Canada;CN=Server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        initData.properties->setProperty("IceSSL.TrustOnly", "!C=Canada;!CN=Server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        initData.properties->setProperty("IceSSL.TrustOnly", "!CN=Server1"); // Should not match "Server"
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.TrustOnly"] = "!CN=Client1"; // Should not match "Client"
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException&)
        {
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
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        CommunicatorPtr comm = initialize(initData);
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.TrustOnly"] = "C=US, ST=Florida, O=ZeroC\\, Inc.,OU=Ice, emailAddress=info@zeroc.com, CN=Client";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.VerifyPeer"] = "0";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const LocalException&)
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
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        CommunicatorPtr comm = initialize(initData);
        initData.properties->setProperty("IceSSL.VerifyPeer", "0");

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.TrustOnly"] = "!C=US, ST=Florida, O=ZeroC\\, Inc.,OU=Ice, emailAddress=info@zeroc.com, CN=Client";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.VerifyPeer"] = "0";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const LocalException&)
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
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        initData.properties->setProperty("IceSSL.TrustOnly", "ST=Florida;!CN=Server;C=US");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const LocalException&)
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
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.TrustOnly"] = "C=US;!CN=Client;ST=Florida";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    cout << "ok" << endl;

    cout << "testing IceSSL.TrustOnly.Client... " << flush;
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        initData.properties->setProperty("IceSSL.TrustOnly.Client", "C=US, ST=Florida, O=ZeroC\\, Inc.,"
                                         "OU=Ice, emailAddress=info@zeroc.com, CN=Server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        // Should have no effect.
        d["IceSSL.TrustOnly.Client"] = "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com,"
                                       "CN=Server";

        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        initData.properties->setProperty("IceSSL.TrustOnly.Client", "!C=US, ST=Florida, O=ZeroC\\, Inc.,"
                                         "OU=Ice, emailAddress=info@zeroc.com, CN=Server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        // Should have no effect.
        d["IceSSL.TrustOnly.Client"] = "!CN=Client";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        initData.properties->setProperty("IceSSL.TrustOnly.Client", "CN=Client");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        initData.properties->setProperty("IceSSL.TrustOnly.Client", "!CN=Client");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    cout << "ok" << endl;

    cout << "testing IceSSL.TrustOnly.Server... " << flush;
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        // Should have no effect.
        initData.properties->setProperty("IceSSL.TrustOnly.Server", "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice,"
                                         "emailAddress=info@zeroc.com,CN=Client");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.TrustOnly.Server"] = "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com,"
                                       "CN=Client";

        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.TrustOnly.Server"] =
            "!C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Client";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        // Should have no effect.
        initData.properties->setProperty("IceSSL.TrustOnly.Server", "!CN=Server");
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.TrustOnly.Server"] = "CN=Server";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.TrustOnly.Server"] = "!CN=Client";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    cout << "ok" << endl;

    cout << "testing IceSSL.TrustOnly.Server.<AdapterName>... " << flush;
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.TrustOnly.Server.ServerAdapter"] =
            "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com,CN=Client";
        d["IceSSL.TrustOnly.Server"] = "CN=bogus";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.TrustOnly.Server.ServerAdapter"] =
            "!C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Client";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.TrustOnly.Server.ServerAdapter"] = "CN=bogus";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
            test(false);
        }
        catch(const LocalException&)
        {
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    {
        InitializationData initData;
        initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
        initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
        if(pfx)
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_ca1.pfx");
        }
        else
        {
            initData.properties->setProperty("IceSSL.CertFile", "c_rsa_nopass_ca1_pub.pem");
            initData.properties->setProperty("IceSSL.KeyFile", "c_rsa_nopass_ca1_priv.pem");
        }
        CommunicatorPtr comm = initialize(initData);

        Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
        test(fact);
        Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
        d["IceSSL.CertAuthFile"] = "cacert1.pem";
        if(pfx)
        {
            d["IceSSL.CertFile"] = "s_rsa_ca1.pfx";
        }
        else
        {
            d["IceSSL.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
            d["IceSSL.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
        }
        d["IceSSL.TrustOnly.Server.ServerAdapter"] = "!CN=bogus";
        Test::ServerPrx server = fact->createServer(d);
        try
        {
            server->ice_ping();
        }
        catch(const LocalException&)
        {
            test(false);
        }
        fact->destroyServer(server);
        comm->destroy();
    }
    cout << "ok" << endl;

    {
#if defined(ICE_USE_SCHANNEL)
        cerr << "testing IceSSL.FindCert... " << flush;
        const char* clientFindCertProperties[] =
        {
            "SUBJECTDN:'CN = Client, E = info@zeroc.com, OU = Ice, O = \"ZeroC, Inc.\", S = Florida, C = US'",
            "ISSUER:'ZeroC, Inc.' SUBJECT:Client SERIAL:02",
            "ISSUERDN:'E = info@zeroc.com, CN = \"ZeroC Test CA 1\", OU = Ice, O = \"ZeroC, Inc.\","
                " L = \"Palm Beach Gardens\", S = Florida, C = US' SUBJECT:Client",
            "THUMBPRINT:'54 26 20 f0 93 a9 b6 bc 2a 8c 83 ef 14 d4 49 18 a3 18 67 46'",
            "SUBJECTKEYID:'58 77 81 07 55 2a 0c 10 19 88 13 47 6f 27 6e 21 75 5f 85 ca'",
            0
        };

        const char* serverFindCertProperties[] =
        {
            "SUBJECTDN:'CN = Server, E = info@zeroc.com, OU = Ice, O = \"ZeroC, Inc.\", S = Florida, C = US'",
            "ISSUER:'ZeroC, Inc.' SUBJECT:Server SERIAL:01",
            "ISSUERDN:'E = info@zeroc.com, CN = \"ZeroC Test CA 1\", OU = Ice, O = \"ZeroC, Inc.\","
                " L = \"Palm Beach Gardens\", S = Florida, C = US' SUBJECT:Server",
            "THUMBPRINT:'27 e0 18 c9 23 12 6c f0 5c da fa 36 5a 4c 63 5a e2 53 07 1a'",
            "SUBJECTKEYID:'a6 42 aa 17 04 41 86 56 67 e4 04 64 59 34 30 c7 4c 6b ef a4'",
            0
        };

        const char* failFindCertProperties[] =
        {
            "nolabel",
            "unknownlabel:foo",
            "LABEL:",
            "SUBJECTDN:'CN = Client, E = infox@zeroc.com, OU = Ice, O = \"ZeroC, Inc.\", S = Florida, C = US'",
            "ISSUER:'ZeroC, Inc.' SUBJECT:Client SERIAL:'02 02'",
            "ISSUERDN:'E = info@zeroc.com, CN = \"ZeroC Test CA 1\", OU = Ice, O = \"ZeroC, Inc.\","
                " L = \"Palm Beach Gardens\", S = Florida, C = ES' SUBJECT:Client",
            "THUMBPRINT:'27 e0 18 c9 23 12 6c f0 5c da fa 36 5a 4c 63 5a e2 53 07 ff'",
            "SUBJECTKEYID:'a6 42 aa 17 04 41 86 56 67 e4 04 64 59 34 30 c7 4c 6b ef ff'",
            0
        };

        const char* certificates[] = {"/s_rsa_ca1.pfx", "/c_rsa_ca1.pfx", 0};

        //
        // First we need to import some certificates in the user store.
        //
        HCERTSTORE store = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, 0, CERT_SYSTEM_STORE_CURRENT_USER, L"MY");
        test(store);

        vector<HCERTSTORE> stores;
        vector<PCCERT_CONTEXT> certs;

        for(int i = 0; certificates[i] != 0; ++i)
        {
            vector<char> buffer;
            readFile(defaultDir + certificates[i], buffer);

            CRYPT_DATA_BLOB pfxBlob;
            pfxBlob.cbData = static_cast<DWORD>(buffer.size());
            pfxBlob.pbData = reinterpret_cast<BYTE*>(&buffer[0]);

            HCERTSTORE pfx = PFXImportCertStore(&pfxBlob, L"password", CRYPT_USER_KEYSET);

            PCCERT_CONTEXT next = 0;
            PCCERT_CONTEXT newCert = 0;
            do
            {
                if((next = CertFindCertificateInStore(pfx, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0,
                                                      CERT_FIND_ANY, 0, next)))
                {
                    if(CertAddCertificateContextToStore(store, next, CERT_STORE_ADD_ALWAYS, &newCert))
                    {
                        certs.push_back(newCert);
                    }
                }
            }
            while(next);

            stores.push_back(pfx);
        }

        for(int i = 0; clientFindCertProperties[i] != 0; i++)
        {
            InitializationData initData;
            initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
            initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
            initData.properties->setProperty("IceSSL.FindCert.CurrentUser.My", clientFindCertProperties[i]);
            //
            // Use TrustOnly to ensure the peer has pick the expected certificate.
            //
            initData.properties->setProperty("IceSSL.TrustOnly", "CN=Server");

            CommunicatorPtr comm = initialize(initData);

            Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
            test(fact);
            Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
            d["IceSSL.CertAuthFile"] = "cacert1.pem";
            d["IceSSL.FindCert.CurrentUser.My"] = serverFindCertProperties[i];
            //
            // Use TrustOnly to ensure the peer has pick the expected certificate.
            //
            d["IceSSL.TrustOnly"] = "CN=Client";

            Test::ServerPrx server = fact->createServer(d);
            try
            {
                server->ice_ping();
            }
            catch(const LocalException& ex)
            {
                cerr << ex << endl;
                findCertsCleanup(store, stores, certs);
                test(false);
            }
            fact->destroyServer(server);
            comm->destroy();
        }

        //
        // These must fail because the search criteria does not match any certificates.
        //
        for(int i = 0; failFindCertProperties[i] != 0; i++)
        {
            InitializationData initData;
            initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
            initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
            initData.properties->setProperty("IceSSL.FindCert.CurrentUser.My", failFindCertProperties[i]);
            try
            {
                CommunicatorPtr comm = initialize(initData);
                findCertsCleanup(store, stores, certs);
                test(false);
            }
            catch(const PluginInitializationException&)
            {
                // expected
            }
            catch(const Ice::LocalException& ex)
            {
                cerr << ex << endl;
                findCertsCleanup(store, stores, certs);
                test(false);
            }
        }

        findCertsCleanup(store, stores, certs);

        //
        // These must fail because we have already remove the certificates.
        //
        for(int i = 0; clientFindCertProperties[i] != 0; i++)
        {
            InitializationData initData;
            initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
            initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
            initData.properties->setProperty("IceSSL.FindCert.CurrentUser.My", clientFindCertProperties[i]);
            try
            {
                CommunicatorPtr comm = initialize(initData);
                test(false);
            }
            catch(const PluginInitializationException&)
            {
                //expected
            }
            catch(const Ice::LocalException& ex)
            {
                cerr << ex << endl;
                test(false);
            }
        }
        cerr << "ok" << endl;
#elif defined(ICE_USE_SECURE_TRANSPORT)
        cerr << "testing IceSSL.FindCert... " << flush;
        const char* clientFindCertProperties[] =
        {
            "SUBJECT:Client",
            "LABEL:'Client'",
            "SUBJECTKEYID:'58 77 81 07 55 2a 0c 10 19 88 13 47 6f 27 6e 21 75 5f 85 ca'",
            "SERIAL:02",
            "SERIAL:02 LABEL:Client",
            0
        };

        const char* serverFindCertProperties[] =
        {
            "SUBJECT:Server",
            "LABEL:'Server'",
            "SUBJECTKEYID:'a6 42 aa 17 04 41 86 56 67 e4 04 64 59 34 30 c7 4c 6b ef a4'",
            "SERIAL:01",
            "SERIAL:01 LABEL:Server",
            0
        };

        const char* failFindCertProperties[] =
        {
            "nolabel",
            "unknownlabel:foo",
            "LABEL:",
            "SUBJECT:ServerX",
            "LABEL:'ServerX'",
            "SUBJECTKEYID:'a6 42 aa 17 04 41 86 56 67 e4 04 64 59 34 30 c7 4c 6b ef ff'",
            "SERIAL:03",
            "SERIAL:01 LABEL:Client",
            0
        };

        for(int i = 0; clientFindCertProperties[i] != 0; i++)
        {
            InitializationData initData;
            initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
            initData.properties->setProperty("IceSSL.CertAuthFile", "cacert1.pem");
            initData.properties->setProperty("IceSSL.Keychain", "../certs/Find.keychain");
            initData.properties->setProperty("IceSSL.KeychainPassword", "password");
            initData.properties->setProperty("IceSSL.FindCert", clientFindCertProperties[i]);
            //
            // Use TrustOnly to ensure the peer has pick the expected certificate.
            //
            initData.properties->setProperty("IceSSL.TrustOnly", "CN=Server");

            CommunicatorPtr comm = initialize(initData);

            Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
            test(fact);
            Test::Properties d = createServerProps(defaultProperties, defaultDir, defaultHost, pfx);
            d["IceSSL.CertAuthFile"] = "cacert1.pem";
            d["IceSSL.Keychain"] = "../certs/Find.keychain";
            d["IceSSL.KeychainPassword"] = "password";
            d["IceSSL.FindCert"] = serverFindCertProperties[i];
            //
            // Use TrustOnly to ensure the peer has pick the expected certificate.
            //
            d["IceSSL.TrustOnly"] = "CN=Client";

            Test::ServerPrx server = fact->createServer(d);
            try
            {
                server->ice_ping();
            }
            catch(const LocalException& ex)
            {
                cerr << ex << endl;
                test(false);
            }
            fact->destroyServer(server);
            comm->destroy();
        }

        for(int i = 0; failFindCertProperties[i] != 0; i++)
        {
            InitializationData initData;
            initData.properties = createClientProps(defaultProperties, defaultDir, defaultHost, pfx);
            initData.properties->setProperty("IceSSL.Keychain", "../certs/Find.keychain");
            initData.properties->setProperty("IceSSL.KeychainPassword", "password");
            initData.properties->setProperty("IceSSL.FindCert", failFindCertProperties[i]);
            try
            {
                CommunicatorPtr comm = initialize(initData);
                test(false);
            }
            catch(const PluginInitializationException&)
            {
                // expected
            }
            catch(const Ice::LocalException& ex)
            {
                cerr << ex << endl;
                test(false);
            }
        }
        cerr << "ok" << endl;
#endif
    }

    if(shutdown)
    {
        factory->shutdown();
    }
}
