// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_PLUGIN_I_H
#define ICE_SSL_PLUGIN_I_H

#include <IceSSL/Plugin.h>
#include <IceSSL/SSLEngineF.h>
#include <Ice/CommunicatorF.h>

namespace IceSSL
{

class PluginI : public IceSSL::Plugin
{
public:

    PluginI(const Ice::CommunicatorPtr&);

    //
    // From Ice::Plugin.
    //
    virtual void initialize();
    virtual void destroy();
    virtual std::string getEngineName() const;
    virtual Ice::Long getEngineVersion() const;

    //
    // From IceSSL::Plugin.
    //
#ifdef ICE_CPP11_MAPPING
    virtual void setCertificateVerifier(std::function<bool(const std::shared_ptr<NativeConnectionInfo>&)>);
    virtual void setPasswordPrompt(std::function<std::string()>);
#else
    virtual void setCertificateVerifier(const CertificateVerifierPtr&);
    virtual void setPasswordPrompt(const PasswordPromptPtr&);
#endif

#ifdef ICE_USE_OPENSSL
    virtual void setContext(SSL_CTX*);
    virtual SSL_CTX* getContext();
#endif

private:

#if defined(ICE_USE_SECURE_TRANSPORT)
    SecureTransportEnginePtr _engine;
#elif defined(ICE_USE_SCHANNEL)
    SChannelEnginePtr _engine;
#elif defined(ICE_OS_UWP)
    UWPEnginePtr _engine;
#else
    OpenSSLEnginePtr _engine;
#endif
};

}

#endif
