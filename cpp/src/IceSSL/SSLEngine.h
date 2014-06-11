// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_ENGINE_H
#define ICE_SSL_ENGINE_H

#include <IceSSL/Plugin.h>
#include <IceSSL/Util.h>
#include <IceSSL/SSLEngineF.h>
#include <IceSSL/TrustManagerF.h>

#include <IceUtil/ScopedArray.h>
#include <IceUtil/UniquePtr.h>
#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <Ice/CommunicatorF.h>
#include <Ice/Network.h>

#ifdef ICE_USE_SECURE_TRANSPORT
#   include <Security/Security.h>
#endif

namespace IceSSL
{

class SSLEngine : public IceUtil::Shared
{
public:

    SSLEngine(const Ice::CommunicatorPtr&);

    Ice::CommunicatorPtr communicator() const { return _communicator; }
    Ice::LoggerPtr getLogger() const { return _logger; };

    void setCertificateVerifier(const CertificateVerifierPtr&);
    void setPasswordPrompt(const PasswordPromptPtr&);
    std::string password(bool);

    //
    // Setup the engine.
    //
    virtual void initialize() = 0;
    
    virtual bool initialized() const = 0;

    //
    // Destroy the engine.
    //
    virtual void destroy() = 0;
    
    //
    // Verify peer certificate
    //
    virtual void verifyPeer(SOCKET, const std::string&, const NativeConnectionInfoPtr&);


    CertificateVerifierPtr getCertificateVerifier() const;
    PasswordPromptPtr getPasswordPrompt() const;

    std::string getPassword() const;
    void setPassword(const std::string& password);
    
    int getVerifyPeer() const { return _verifyPeer; }
    int securityTraceLevel() const { return _securityTraceLevel; }
    std::string securityTraceCategory() const { return _securityTraceCategory; }
    
private:
    
    const Ice::CommunicatorPtr _communicator;
    const Ice::LoggerPtr _logger;
    const TrustManagerPtr _trustManager;
    
    std::string _password;
    CertificateVerifierPtr _verifier;
    PasswordPromptPtr _prompt;
    
    bool _checkCertName;
    int _verifyDepthMax;
    int _verifyPeer;
    int _securityTraceLevel;
    std::string _securityTraceCategory;
};

#ifdef ICE_USE_SECURE_TRANSPORT

class SecureTransportEngine : public SSLEngine
{
public:
    
    SecureTransportEngine(const Ice::CommunicatorPtr&);
    
    virtual void initialize();
    virtual bool initialized() const;
    virtual void destroy();
    
    SSLContextRef newContext(bool);
    CFArrayRef getCertificateAuthorities() const;
    std::string getCipherName(SSLCipherSuite) const;
    SecCertificateRef getCertificate() const;
    SecKeychainRef getKeychain() const;
    
private:
    
    void parseCiphers(const std::string& ciphers);
    
    bool _initialized;
    SSLContextRef _ctx;
    CFArrayRef _certificateAuthorities;
    SecCertificateRef _cert;
    SecKeyRef _key;
    SecIdentityRef _identity;
    SecKeychainRef _keychain;
    
    SSLProtocol _protocolVersionMax;
    SSLProtocol _protocolVersionMin;
    
    std::string _defaultDir;
    
   
    IceUtil::UniquePtr< IceUtil::ScopedArray<char> > _dhParams;
    size_t _dhParamsLength;
    
    std::vector<SSLCipherSuite> _ciphers;
    bool _allCiphers;
    IceUtil::Mutex _mutex;
};

#else
class OpenSSLEngine : public SSLEngine
{
public:
    
    OpenSSLEngine(const Ice::CommunicatorPtr&);
    ~OpenSSLEngine();
    
    virtual void initialize();
    virtual bool initialized() const;
    virtual void destroy();
    virtual void verifyPeer(SSL*, SOCKET, const std::string&, const NativeConnectionInfoPtr&);
    virtual void traceConnection();
    
    int verifyCallback(int , SSL*, X509_STORE_CTX*);
#  ifndef OPENSSL_NO_DH
    DH* dhParams(int);
#  endif
    SSL_CTX* context() const;
    void context(SSL_CTX*);
    std::string sslErrors() const;
    
private:
    
    SSL_METHOD* getMethod(int);
    void setOptions(int);
    enum Protocols { SSLv3 = 0x01, TLSv1_0 = 0x02, TLSv1_1 = 0x04, TLSv1_2 = 0x08 };
    int parseProtocols(const Ice::StringSeq&) const;


    bool _initOpenSSL;
    bool _initialized;
    SSL_CTX* _ctx;
    std::string _defaultDir;

#  ifndef OPENSSL_NO_DH
    DHParamsPtr _dhParams;
#  endif
    IceUtil::Mutex _mutex;
};
#endif

}

#endif
