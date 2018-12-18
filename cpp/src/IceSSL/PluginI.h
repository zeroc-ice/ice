// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESSL_PLUGIN_I_H
#define ICESSL_PLUGIN_I_H

#include <IceSSL/Plugin.h>
#include <IceSSL/SSLEngineF.h>
#include <Ice/CommunicatorF.h>

namespace IceSSL
{

class ICESSL_API PluginI : public virtual IceSSL::Plugin
{
public:

    PluginI(const Ice::CommunicatorPtr&, const IceSSL::SSLEnginePtr&);
    //
    // From Ice::Plugin.
    //
    virtual void initialize();
    virtual void destroy();

    //
    // From IceSSL::Plugin.
    //
#ifdef ICE_CPP11_MAPPING
    virtual void setCertificateVerifier(std::function<bool(const std::shared_ptr<ConnectionInfo>&)>);
    virtual void setPasswordPrompt(std::function<std::string()>);
#else
    virtual void setCertificateVerifier(const CertificateVerifierPtr&);
    virtual void setPasswordPrompt(const PasswordPromptPtr&);
#endif

    virtual CertificatePtr load(const std::string&) const = 0;
    virtual CertificatePtr decode(const std::string&) const = 0;
protected:

    SSLEnginePtr _engine;
};

}

#endif
