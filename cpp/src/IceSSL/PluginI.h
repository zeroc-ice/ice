// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_PLUGIN_I_H
#define ICE_SSL_PLUGIN_I_H

#include <IceSSL/Plugin.h>
#include <InstanceF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/Plugin.h>

namespace IceSSL
{

class PluginI : public IceSSL::Plugin
{
public:

    PluginI(const Ice::CommunicatorPtr&);

    virtual void destroy();

    virtual void initialize(SSL_CTX* = 0, SSL_CTX* = 0);
    virtual void setCertificateVerifier(const CertificateVerifierPtr&);
    virtual void setPasswordPrompt(const PasswordPromptPtr&);

    virtual SSL_CTX* clientContext();
    virtual SSL_CTX* serverContext();

private:

    void setupSSL(const Ice::CommunicatorPtr&);
    void cleanupSSL();

    InstancePtr _instance;
};

}

#endif
