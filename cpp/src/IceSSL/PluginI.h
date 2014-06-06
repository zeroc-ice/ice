// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

class PluginI : 
#ifdef ICE_USE_OPENSSL
    public OpenSSLPlugin
#else
    public IceSSL::Plugin
#endif
{
public:

    PluginI(const Ice::CommunicatorPtr&);

    //
    // From Ice::Plugin.
    //
    virtual void initialize();
    virtual void destroy();

    //
    // From IceSSL::Plugin.
    //
    virtual void setCertificateVerifier(const CertificateVerifierPtr&);
    virtual void setPasswordPrompt(const PasswordPromptPtr&);

#ifdef ICE_USE_OPENSSL
    virtual void setContext(ContextRef);
    virtual ContextRef getContext();
#endif

private:

#ifdef ICE_USE_OPENSSL
    OpenSSLEnginePtr _engine;
#else
    SecureTransportEnginePtr _engine;
#endif
};

}

#endif
