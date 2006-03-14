// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_OPENSSL_PLUGIN_I_H
#define ICE_SSL_OPENSSL_PLUGIN_I_H

#include <IceUtil/RecMutex.h>

#include <Ice/LoggerF.h>
#include <Ice/StatsF.h>
#include <Ice/PropertiesF.h>
#include <Ice/ProtocolPluginFacadeF.h>

#include <IceSSL/OpenSSLPluginIF.h>
#include <IceSSL/CertificateVerifierF.h>
#include <IceSSL/TraceLevelsF.h>
#include <IceSSL/SslTransceiverF.h>
#include <IceSSL/RSAPrivateKeyF.h>

#include <IceSSL/Plugin.h>
#include <IceSSL/CertificateDesc.h>
#include <IceSSL/CertificateAuthority.h>
#include <IceSSL/BaseCerts.h>
#include <IceSSL/TempCerts.h>
#include <IceSSL/ServerContext.h>
#include <IceSSL/ClientContext.h>
#include <IceSSL/DHParamsF.h>

#include <openssl/ssl.h>

namespace IceSSL
{

typedef std::map<int,RSAPrivateKeyPtr> RSAMap;
typedef std::map<int,DHParamsPtr> DHMap;

typedef std::map<int,CertificateDesc> RSACertMap;
typedef std::map<int,DiffieHellmanParamsFile> DHParamsMap;

class OpenSSLPluginI : public Plugin
{
public:

    OpenSSLPluginI(const IceInternal::ProtocolPluginFacadePtr&);
    virtual ~OpenSSLPluginI();

    SslTransceiverPtr createServerTransceiver(int, int);
    SslTransceiverPtr createClientTransceiver(int, int);

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

    virtual void setCertificateVerifier(ContextType, const CertificateVerifierPtr&);
    virtual void addTrustedCertificateBase64(ContextType, const std::string&);
    virtual void addTrustedCertificate(ContextType, const Ice::ByteSeq&);
    virtual void setRSAKeysBase64(ContextType, const std::string&, const std::string&);
    virtual void setRSAKeys(ContextType, const ::Ice::ByteSeq&, const ::Ice::ByteSeq&);

    virtual IceSSL::CertificateVerifierPtr getDefaultCertVerifier();
    virtual IceSSL::CertificateVerifierPtr getSingleCertVerifier(const Ice::ByteSeq&);
    virtual void destroy();

    TraceLevelsPtr getTraceLevels() const;
    Ice::LoggerPtr getLogger() const;
    Ice::StatsPtr getStats() const;
    Ice::PropertiesPtr getProperties() const;
    IceInternal::ProtocolPluginFacadePtr getProtocolPluginFacade() const;

private:

    const IceInternal::ProtocolPluginFacadePtr _protocolPluginFacade;
    const TraceLevelsPtr _traceLevels;
    const Ice::PropertiesPtr _properties;
    const int _memDebug;

    IceSSL::ServerContext _serverContext;
    IceSSL::ClientContext _clientContext;
    
    // Mutex to ensure synchronization of calls to configure
    // the contexts and calls to create connections.
    IceUtil::RecMutex _configMutex;

    // Keep a cache of all temporary RSA keys.
    RSAMap _tempRSAKeys;
    IceUtil::Mutex _tempRSAKeysMutex;

    // Keep a cache of all temporary Diffie-Hellman keys.
    DHMap _tempDHKeys;
    IceUtil::Mutex _tempDHKeysMutex;

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
    void loadTempCerts(TempCertificates&);

    friend class SslTransceiver;
    friend class SslClientTransceiver;
    friend class SslServerTransceiver;

    IceUtil::Mutex _threadIdCacheMutex;
    std::vector<unsigned long> _threadIdCache;

    void registerThread();
    void unregisterThreads();
};

}

#endif
