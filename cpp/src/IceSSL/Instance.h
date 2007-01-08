// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
#include <Ice/LoggerF.h>
#include <Ice/Network.h>
#include <Ice/ProtocolPluginFacadeF.h>
#include <IceSSL/Plugin.h>
#include <IceSSL/TrustManagerF.h>

namespace IceSSL
{

class Instance : public IceUtil::Shared
{
public:

    Instance(const Ice::CommunicatorPtr&);

    void initialize();
    void context(SSL_CTX*);
    SSL_CTX* context() const;
    void setCertificateVerifier(const CertificateVerifierPtr&);
    void setPasswordPrompt(const PasswordPromptPtr&);

    Ice::CommunicatorPtr communicator() const;
    std::string defaultHost() const;
    int networkTraceLevel() const;
    std::string networkTraceCategory() const;
    int securityTraceLevel() const;
    std::string securityTraceCategory() const;

    void verifyPeer(SSL*, SOCKET, const std::string&, const std::string&, bool);

    std::string sslErrors() const;

    void traceConnection(SSL*, bool);

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

    void parseProtocols(const std::string&);

    Ice::LoggerPtr _logger;
    IceInternal::ProtocolPluginFacadePtr _facade;
    int _securityTraceLevel;
    std::string _securityTraceCategory;
    SSL_CTX* _ctx;
    std::string _defaultDir;
    bool _checkCertName;
    int _verifyDepthMax;
    std::string _password;
#ifndef OPENSSL_NO_DH
    DHParamsPtr _dhParams;
#endif
    CertificateVerifierPtr _verifier;
    PasswordPromptPtr _prompt;
    TrustManagerPtr _trustManager;
};

}

#endif
