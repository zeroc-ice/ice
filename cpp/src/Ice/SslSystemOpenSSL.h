// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************
#ifndef ICE_SSL_SYSTEM_OPENSSL_H
#define ICE_SSL_SYSTEM_OPENSSL_H

#include <openssl/ssl.h>
#include <string>
#include <map>
#include <Ice/Config.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/SslFactory.h>
#include <Ice/SslSystem.h>

namespace IceSecurity
{

namespace Ssl
{

enum SslProtocol
{
    SSL_V2 = 1,     // Only speak SSLv2
    SSL_V23,        // Speak SSLv2 and SSLv3
    SSL_V3,         // Only speak SSLv3
    TLS_V1          // Only speak TLSv1
};

}

}

#include <Ice/SslGeneralConfig.h>
#include <Ice/SslCertificateDesc.h>
#include <Ice/SslCertificateAuthority.h>
#include <Ice/SslBaseCerts.h>
#include <Ice/SslTempCerts.h>

extern "C"
{
    RSA* tmpRSACallback(SSL*, int, int);
    DH* tmpDHCallback(SSL*, int, int);
}

namespace IceSecurity
{

namespace Ssl
{

class GeneralConfig;

namespace OpenSSL
{

using namespace Ice;
using namespace IceSecurity::Ssl;

using std::map;
using std::string;

typedef map<int,RSA*> RSAMap;
typedef map<int,DH*>  DHMap;

typedef map<int,CertificateDesc> RSACertMap;
typedef map<int,CertificateDesc> DSACertMap;
typedef map<int,DiffieHellmanParamsFile> DHParamsMap;

class System : public IceSecurity::Ssl::System
{
public:

    void printContextInfo(SSL_CTX*);

    // This is how we create a Server connection.
    virtual IceSecurity::Ssl::Connection* createServerConnection(int);

    // This is how we create a Client connection.
    virtual IceSecurity::Ssl::Connection* createClientConnection(int);

    // Shuts down the SSL System.
    virtual void shutdown();

    virtual bool isConfigLoaded();
    virtual void loadConfig();

    // Returns the desired RSA Key, or creates it if not already created.
    // This is public because the tmpRSACallback must be able to access it.
    RSA* getRSAKey(SSL*, int, int);

    // Returns the desired DH Params. If the Params do not already exist, and the key
    // requested is a 512bit or 1024bit key, we use the compiled-in temporary params.
    // If the key is some other length, we read the desired key, based on length,
    // from a DH Param file. 
    // This is public because the tmpDHCallback must be able to access it.
    DH* getDHParams(SSL*, int, int);

    static TraceLevelsPtr _globalTraceLevels;
    static Ice::LoggerPtr _globalLogger;

protected:

    System();
    ~System();
    
private:
    
    // Base Diffie-Hellman 512bit key (only to be used for key exchange).
    static unsigned char _tempDiffieHellman512p[];
    static unsigned char _tempDiffieHellman512g[];
    
    // Default SSL Contexts, for both Server and Client connections.
    SSL_CTX* _sslServerContext;
    SSL_CTX* _sslClientContext;

    // Keep a cache of all temporary RSA keys.
    RSAMap _tempRSAKeys;
    ::IceUtil::Mutex _tempRSAKeysMutex;

    // Keep a cache of all temporary Diffie-Hellman keys.
    DHMap _tempDHKeys;
    ::IceUtil::Mutex _tempDHKeysMutex;

    // Maps of all temporary keying information.
    // The files themselves will not be loaded until
    // needed.
    RSACertMap _tempRSAFileMap;
    DSACertMap _tempDSAFileMap;
    DHParamsMap _tempDHParamsFileMap;

    // The Session ID Context (Server Only).
    string _sessionContext;

    // Flag as to whether the Random Number system has been seeded.
    int _randSeeded;

    bool _configLoaded;

    void setKeyCert(SSL_CTX*, const CertificateDesc&, const string&, const string&);

    // Call to initialize the SSL system.
    void initClient(GeneralConfig&, CertificateAuthority&, BaseCertificates&);
    void initServer(GeneralConfig&, CertificateAuthority&, BaseCertificates&, TempCertificates&);

    SSL_METHOD* getSslMethod(SslProtocol);

    void processCertificate(SSL_CTX*, const CertificateDesc&);
    void addKeyCert(SSL_CTX*, const CertificateFile&, const CertificateFile&);
    void addKeyCert(SSL_CTX*, const string&, const string&);

    SSL_CTX* createContext(SslProtocol);

    // Retrieves errors from the OpenSSL library.
    string sslGetErrors();

    void commonConnectionSetup(Connection*);

    // Create a connection.
    SSL* createConnection(SSL_CTX*, int);

    // Methods for loading CAFiles into a Context.
    void loadCAFiles(SSL_CTX*, CertificateAuthority&);
    void loadCAFiles(SSL_CTX*, const char*, const char*);
    void loadAndCheckCAFiles(SSL_CTX*, CertificateAuthority&);

    DH* loadDHParam(const char *);
    DH* getTempDH(unsigned char*, int, unsigned char*, int);
    DH* getTempDH512();
    void setDHParams(SSL_CTX*, BaseCertificates&);

    void setCipherList(SSL_CTX*, const string&);

    // Cryptographic Random Number System related routines.
    int seedRand();
    long loadRandFiles(const string&);
    void initRandSystem(const string&);

    void loadTempCerts(TempCertificates&);

    friend class IceSecurity::Ssl::Factory;
    friend class Connection;
};

}

}

}

#endif
