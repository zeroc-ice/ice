// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_INSTANCE_H
#define ICE_SSL_INSTANCE_H

#include <IceSSL/InstanceF.h>
#include <IceSSL/UtilF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/Network.h>
#include <Ice/ProtocolInstance.h>
#include <Ice/ProtocolPluginFacadeF.h>
#include <IceSSL/Plugin.h>
#include <IceSSL/TrustManagerF.h>
#include <Ice/BuiltinSequences.h>

namespace IceSSL
{

class SharedInstance : public IceUtil::Shared
{
public:

    SharedInstance(const Ice::CommunicatorPtr&);
    ~SharedInstance();

    void initialize();
    void context(SSL_CTX*);
    SSL_CTX* context() const;
    void setCertificateVerifier(const CertificateVerifierPtr&);
    void setPasswordPrompt(const PasswordPromptPtr&);

    Ice::CommunicatorPtr communicator() const;

    void verifyPeer(SSL*, SOCKET, const std::string&, const NativeConnectionInfoPtr&);

    std::string sslErrors() const;

    void destroy();

    //
    // OpenSSL callbacks.
    //
    std::string password(bool);
    int verifyCallback(int, SSL*, X509_STORE_CTX*);
#ifndef OPENSSL_NO_DH
    DH* dhParams(int);
#endif

private:

    enum Protocols { SSLv3 = 0x01, TLSv1_0 = 0x02, TLSv1_1 = 0x04, TLSv1_2 = 0x08 };
    static int parseProtocols(const Ice::StringSeq&);

    static SSL_METHOD* getMethod(int);

    void setOptions(int);

    bool _initOpenSSL;
    const Ice::CommunicatorPtr _communicator;
    const Ice::LoggerPtr _logger;
    int _securityTraceLevel;
    std::string _securityTraceCategory;
    bool _initialized;
    SSL_CTX* _ctx;
    std::string _defaultDir;
    bool _checkCertName;
    int _verifyDepthMax;
    int _verifyPeer;
    std::string _password;
#ifndef OPENSSL_NO_DH
    DHParamsPtr _dhParams;
#endif
    CertificateVerifierPtr _verifier;
    PasswordPromptPtr _prompt;
    TrustManagerPtr _trustManager;
};

class Instance : public IceInternal::ProtocolInstance
{
public:

    Instance(const SharedInstancePtr&, Ice::Short, const std::string&);
    virtual ~Instance();

    SSL_CTX* 
    context() const
    {
        return _sharedInstance->context();
    }

    std::string 
    sslErrors() const
    {
        return _sharedInstance->sslErrors();
    }

    SharedInstancePtr 
    sharedInstance() const
    {
        return _sharedInstance;
    }

    void 
    verifyPeer(SSL* ssl, SOCKET fd, const std::string& host, const NativeConnectionInfoPtr& info)
    {
        _sharedInstance->verifyPeer(ssl, fd, host, info);
    }

    void traceConnection(SSL*, bool);
    int securityTraceLevel() const;
    std::string securityTraceCategory() const;

private:

    const SharedInstancePtr _sharedInstance;
    int _securityTraceLevel;
    std::string _securityTraceCategory;
};

}

#endif
