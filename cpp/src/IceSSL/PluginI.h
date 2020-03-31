//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
    virtual void setCertificateVerifier(std::function<bool(const std::shared_ptr<ConnectionInfo>&)>);
    virtual void setPasswordPrompt(std::function<std::string()>);

    virtual CertificatePtr load(const std::string&) const = 0;
    virtual CertificatePtr decode(const std::string&) const = 0;
protected:

    SSLEnginePtr _engine;
};

}

#endif
