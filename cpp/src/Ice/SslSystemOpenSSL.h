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
#include <Ice/SslConnectionOpenSSL.h>

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

typedef std::map<int,RSA*> RSAMap;
typedef std::map<int,DH*>  DHMap;

typedef std::map<int,CertificateDesc> RSACertMap;
typedef std::map<int,CertificateDesc> DSACertMap;
typedef std::map<int,DiffieHellmanParamsFile> DHParamsMap;

class System : public IceSecurity::Ssl::System
{
public:

    // This is how we create a Server connection.
    virtual IceSecurity::Ssl::Connection* createServerConnection(int);

    // This is how we create a Client connection.
    virtual IceSecurity::Ssl::Connection* createClientConnection(int);

    // Shuts down the SSL System.
    virtual void shutdown();

    virtual void setTrace(const IceInternal::TraceLevelsPtr&);
    virtual void setLogger(const Ice::LoggerPtr&);

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

    CertificateVerifierPtr certificateVerifierTypeCheck(const IceSecurity::Ssl::CertificateVerifierPtr&);
    virtual void setServerCertificateVerifier(const IceSecurity::Ssl::CertificateVerifierPtr&);
    virtual void setClientCertificateVerifier(const IceSecurity::Ssl::CertificateVerifierPtr&);

    virtual void setServerCertAuthorityCertificate(const std::string&);
    virtual void setClientCertAuthorityCertificate(const std::string&);

    virtual void setServerRSAKeysBase64(const std::string&, const std::string&);
    virtual void setClientRSAKeysBase64(const std::string&, const std::string&);

    static IceInternal::TraceLevelsPtr _globalTraceLevels;
    static Ice::LoggerPtr _globalLogger;

protected:

    System();
    ~System();
    
private:
    
    CertificateVerifierPtr _clientVerifier;
    CertificateVerifierPtr _serverVerifier;

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
    std::string _sessionContext;

    // Flag as to whether the Random Number system has been seeded.
    int _randSeeded;

    bool _configLoaded;

    void setKeyCert(SSL_CTX*, const IceSecurity::Ssl::CertificateDesc&,
                              const std::string&, const std::string&);

    // Call to initialize the SSL system.
    void initClient(IceSecurity::Ssl::GeneralConfig&, IceSecurity::Ssl::CertificateAuthority&,
                    IceSecurity::Ssl::BaseCertificates&);
    void initServer(IceSecurity::Ssl::GeneralConfig&, IceSecurity::Ssl::CertificateAuthority&,
                    IceSecurity::Ssl::BaseCertificates&, IceSecurity::Ssl::TempCertificates&);

    SSL_METHOD* getSslMethod(SslProtocol);

    void processCertificate(SSL_CTX*, const IceSecurity::Ssl::CertificateDesc&);
    void addKeyCert(SSL_CTX*, const IceSecurity::Ssl::CertificateFile&,
                              const IceSecurity::Ssl::CertificateFile&);
    void addKeyCert(SSL_CTX*, const std::string&, const std::string&);

    SSL_CTX* createContext(IceSecurity::Ssl::SslProtocol);

    // Retrieves errors from the OpenSSL library.
    std::string sslGetErrors();

    void commonConnectionSetup(IceSecurity::Ssl::OpenSSL::Connection*);

    // Create a connection.
    SSL* createConnection(SSL_CTX*, int);

    // Methods for loading CAFiles into a Context.
    void loadCAFiles(SSL_CTX*, IceSecurity::Ssl::CertificateAuthority&);
    void loadCAFiles(SSL_CTX*, const char*, const char*);
    void loadAndCheckCAFiles(SSL_CTX*, IceSecurity::Ssl::CertificateAuthority&);

    DH* loadDHParam(const char *);
    DH* getTempDH(unsigned char*, int, unsigned char*, int);
    DH* getTempDH512();
    void setDHParams(SSL_CTX*, IceSecurity::Ssl::BaseCertificates&);

    void setCipherList(SSL_CTX*, const std::string&);

    // Cryptographic Random Number System related routines.
    int seedRand();
    long loadRandFiles(const std::string&);
    void initRandSystem(const std::string&);

    void loadTempCerts(IceSecurity::Ssl::TempCertificates&);

    friend class IceSecurity::Ssl::Factory;
    friend class Connection;
};

}

}

}

#endif
