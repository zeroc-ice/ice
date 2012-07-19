// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <IceSSL/Plugin.h>
#include <IceSSL/InstanceF.h>
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

    //
    // From IceSSL::Plugin.
    //
    virtual void setContext(SSL_CTX*);
    virtual SSL_CTX* getContext();
    virtual void setCertificateVerifier(const CertificateVerifierPtr&);
    virtual void setPasswordPrompt(const PasswordPromptPtr&);

private:

    InstancePtr _instance;
};

}
