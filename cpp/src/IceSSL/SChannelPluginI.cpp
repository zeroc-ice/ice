// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/PluginI.h>
#include <IceSSL/SChannelEngine.h>

#include <Ice/Initialize.h>

using namespace std;

namespace
{

class PluginI : public IceSSL::PluginI
{
public:

    PluginI(const Ice::CommunicatorPtr&);

    virtual IceSSL::CertificatePtr create(CERT_SIGNED_CONTENT_INFO*) const;
    virtual IceSSL::CertificatePtr load(const std::string&) const;
    virtual IceSSL::CertificatePtr decode(const std::string&) const;
};

} // anonymous namespace end

//
// Plugin implementation.
//
PluginI::PluginI(const Ice::CommunicatorPtr& com) :
    IceSSL::PluginI(com, new IceSSL::SChannel::SSLEngine(com))
{
}

IceSSL::CertificatePtr
PluginI::create(CERT_SIGNED_CONTENT_INFO* cert) const
{
    return IceSSL::SChannel::Certificate::create(cert);
}

IceSSL::CertificatePtr
PluginI::load(const std::string& file) const
{
    return IceSSL::SChannel::Certificate::load(file);
}

IceSSL::CertificatePtr
PluginI::decode(const std::string& encoding) const
{
    return IceSSL::SChannel::Certificate::load(encoding);
}

//
// Plug-in factory function.
//
extern "C" ICESSL_API Ice::Plugin*
createIceSSL(const Ice::CommunicatorPtr& communicator, const string& /*name*/, const Ice::StringSeq& /*args*/)
{
    return new PluginI(communicator);
}

IceSSL::CertificatePtr
IceSSL::Certificate::load(const std::string& file)
{
    return IceSSL::SChannel::Certificate::load(file);
}

IceSSL::CertificatePtr
IceSSL::Certificate::decode(const std::string& encoding)
{
    return IceSSL::SChannel::Certificate::decode(encoding);
}
