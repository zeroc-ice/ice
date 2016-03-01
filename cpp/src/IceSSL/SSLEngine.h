// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <Ice/CommunicatorF.h>
#include <Ice/Network.h>

#if defined(ICE_USE_SECURE_TRANSPORT)
#   include <Security/Security.h>
#elif defined(ICE_USE_SCHANNEL)

//
// SECURITY_WIN32 or SECURITY_KERNEL, must be defined before including security.h
// indicating who is compiling the code.
//
#  ifdef SECURITY_WIN32
#    undef SECURITY_WIN32
#  endif
#  ifdef SECURITY_KERNEL
#    undef SECURITY_KERNEL
#  endif
#  define SECURITY_WIN32 1
#  include <security.h>
#  include <sspi.h>
#  include <schannel.h>
#  undef SECURITY_WIN32
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
    void verifyPeer(SOCKET, const std::string&, const NativeConnectionInfoPtr&);

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

#if defined(ICE_USE_SECURE_TRANSPORT)

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

private:

    void parseCiphers(const std::string&);
    SecKeychainRef openKeychain();

    bool _initialized;
    UniqueRef<CFArrayRef> _certificateAuthorities;
    UniqueRef<CFArrayRef> _chain;

    SSLProtocol _protocolVersionMax;
    SSLProtocol _protocolVersionMin;

    std::string _defaultDir;

    std::vector<char> _dhParams;

    std::vector<SSLCipherSuite> _ciphers;
    IceUtil::Mutex _mutex;
};

#elif defined(ICE_USE_SCHANNEL)


#if defined(__MINGW32__) || (defined(_MSC_VER) && (_MSC_VER <= 1500))

//
// Add some definitions missing from MinGW headers.
//

#   ifndef CERT_TRUST_IS_EXPLICIT_DISTRUST
#      define CERT_TRUST_IS_EXPLICIT_DISTRUST 0x04000000
#   endif

#   ifndef CERT_TRUST_HAS_NOT_SUPPORTED_CRITICAL_EXT
#      define CERT_TRUST_HAS_NOT_SUPPORTED_CRITICAL_EXT 0x08000000
#   endif

#   ifndef SECBUFFER_ALERT
#      define SECBUFFER_ALERT 17
#   endif

#   ifndef SCH_SEND_ROOT_CERT
#      define SCH_SEND_ROOT_CERT 0x00040000
#   endif

#   ifndef SP_PROT_TLS1_1_SERVER
#      define SP_PROT_TLS1_1_SERVER 0x00000100
#   endif

#   ifndef SP_PROT_TLS1_1_CLIENT
#      define SP_PROT_TLS1_1_CLIENT 0x00000200
#   endif

#   ifndef SP_PROT_TLS1_2_SERVER
#      define SP_PROT_TLS1_2_SERVER 0x00000400
#   endif

#   ifndef SP_PROT_TLS1_2_CLIENT
#      define SP_PROT_TLS1_2_CLIENT 0x00000800
#   endif

#endif
class SChannelEngine : public SSLEngine
{
public:

    SChannelEngine(const Ice::CommunicatorPtr&);

    //
    // Setup the engine.
    //
    virtual void initialize();

    virtual bool initialized() const;

    //
    // Destroy the engine.
    //
    virtual void destroy();

    std::string getCipherName(ALG_ID) const;

    CredHandle newCredentialsHandle(bool);

    HCERTCHAINENGINE chainEngine() const;

private:

    void parseCiphers(const std::string&);

    bool _initialized;
    std::vector<PCCERT_CONTEXT> _allCerts;
    std::vector<PCCERT_CONTEXT> _importedCerts;
    DWORD _protocols;
    IceUtil::Mutex _mutex;

    std::vector<HCERTSTORE> _stores;
    HCERTSTORE _rootStore;

    HCERTCHAINENGINE _chainEngine;
    std::vector<ALG_ID> _ciphers;
};
#else // OpenSSL
class OpenSSLEngine : public SSLEngine
{
public:

    OpenSSLEngine(const Ice::CommunicatorPtr&);
    ~OpenSSLEngine();

    virtual void initialize();
    virtual bool initialized() const;
    virtual void destroy();

#   ifndef OPENSSL_NO_DH
    DH* dhParams(int);
#   endif
    SSL_CTX* context() const;
    void context(SSL_CTX*);
    std::string sslErrors() const;

private:

    SSL_METHOD* getMethod(int);
    void setOptions(int);
    enum Protocols { SSLv3 = 0x01, TLSv1_0 = 0x02, TLSv1_1 = 0x04, TLSv1_2 = 0x08 };
    int parseProtocols(const Ice::StringSeq&) const;


    bool _initialized;
    SSL_CTX* _ctx;
    std::string _defaultDir;

#   ifndef OPENSSL_NO_DH
    DHParamsPtr _dhParams;
#   endif
    IceUtil::Mutex _mutex;
};
#endif

}

#endif
