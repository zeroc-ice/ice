// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_PLUGIN_I_H
#define ICE_SSL_PLUGIN_I_H

#include <IceSSL/Plugin.h>
#include <IceSSL/InstanceF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/Plugin.h>

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

    //
    // From IceSSL::Plugin.
    //
    virtual void setContext(SSL_CTX*);
    virtual SSL_CTX* getContext();
    virtual void setCertificateVerifier(const CertificateVerifierPtr&);
    virtual void setPasswordPrompt(const PasswordPromptPtr&);

private:

    void setupSSL(const Ice::CommunicatorPtr&);
    void cleanupSSL();

    InstancePtr _instance;
};

}

#endif
