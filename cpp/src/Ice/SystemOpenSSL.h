// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************
#ifndef ICE_SSL_SYSTEM_OPENSSL_H
#define ICE_SSL_SYSTEM_OPENSSL_H

#include <Ice/Config.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <IceUtil/RecMutex.h>

#include <Ice/GeneralConfig.h>
#include <Ice/CertificateDesc.h>
#include <Ice/CertificateAuthority.h>
#include <Ice/BaseCerts.h>
#include <Ice/TempCerts.h>

#include <Ice/ContextOpenSSLServer.h>
#include <Ice/ContextOpenSSLClient.h>
#include <Ice/SslConnectionOpenSSL.h>
#include <Ice/SystemInternal.h>
#include <Ice/SslFactory.h>

#include <Ice/OpenSSLUtils.h>
#include <openssl/ssl.h>
#include <string>
#include <map>

#include <Ice/RSAPrivateKeyF.h>
#include <Ice/DHParamsF.h>

namespace IceSSL
{

class GeneralConfig;

namespace OpenSSL
{

typedef std::map<int,RSAPrivateKeyPtr> RSAMap;
typedef std::map<int,DHParamsPtr> DHMap;

typedef std::map<int,CertificateDesc> RSACertMap;
typedef std::map<int,DiffieHellmanParamsFile> DHParamsMap;

class System : public IceSSL::SystemInternal
{
public:

    virtual IceSSL::ConnectionPtr createConnection(ContextType, int);

    virtual bool isConfigured(ContextType);
    virtual void configure();
    virtual void configure(ContextType);
    virtual void loadConfig(ContextType, const ::std::string&, const ::std::string&);

    // Returns the desired RSA Key, or creates it if not already created.
    // This is public because the tmpRSACallback must be able to access it.
    RSA* getRSAKey(int, int);

    // Returns the desired DH Params. If the Params do not already exist, and the key
    // requested is a 512bit or 1024bit key, we use the compiled-in temporary params.
    // If the key is some other length, we read the desired key, based on length,
    // from a DH Param file. 
    // This is public because the tmpDHCallback must be able to access it.
    DH* getDHParams(int, int);

    virtual void setCertificateVerifier(ContextType, const IceSSL::CertificateVerifierPtr&);

    virtual void addTrustedCertificateBase64(ContextType, const std::string&);

    virtual void addTrustedCertificate(ContextType, const Ice::ByteSeq&);

    virtual void setRSAKeysBase64(ContextType, const std::string&, const std::string&);

    virtual void setRSAKeys(ContextType, const Ice::ByteSeq&, const Ice::ByteSeq&);

protected:

    System(const IceInternal::InstancePtr&);
    ~System();
    
private:

    ServerContext _serverContext;
    ClientContext _clientContext;
    
    // Mutex to ensure synchronization of calls to configure
    // the contexts and calls to create connections.
    ::IceUtil::RecMutex _configMutex;

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
    DHParamsMap _tempDHParamsFileMap;

    // Flag as to whether the Random Number system has been seeded.
    int _randSeeded;

    // Cryptographic Random Number System related routines.
    int seedRand();
    long loadRandFiles(const std::string&);
    void initRandSystem(const std::string&);

    // Load the temporary (ephemeral) certificates for Server operations.
    void loadTempCerts(IceSSL::TempCertificates&);

    friend class IceSSL::Factory;
};

}

}

#endif
