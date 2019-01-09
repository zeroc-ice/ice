// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <IceSSL/PluginI.h>
#include <IceSSL/SecureTransport.h>
#include <IceSSL/SecureTransportEngine.h>

#include <Ice/Initialize.h>

using namespace Ice;
using namespace std;

namespace
{

class PluginI : public IceSSL::PluginI
{
public:

    PluginI(const Ice::CommunicatorPtr&);

    virtual IceSSL::CertificatePtr create(SecCertificateRef) const;
    virtual IceSSL::CertificatePtr load(const std::string&) const;
    virtual IceSSL::CertificatePtr decode(const std::string&) const;
};

} // anonymous namespace end

//
// Plugin implementation.
//
PluginI::PluginI(const Ice::CommunicatorPtr& com) :
    IceSSL::PluginI(com, new IceSSL::SecureTransport::SSLEngine(com))
{
}

IceSSL::CertificatePtr
PluginI::create(SecCertificateRef cert) const
{
    return IceSSL::SecureTransport::Certificate::create(cert);
}

IceSSL::CertificatePtr
PluginI::load(const std::string& file) const
{
    return IceSSL::SecureTransport::Certificate::load(file);
}

IceSSL::CertificatePtr
PluginI::decode(const std::string& encoding) const
{
    return IceSSL::SecureTransport::Certificate::load(encoding);
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
    return IceSSL::SecureTransport::Certificate::load(file);
}

IceSSL::CertificatePtr
IceSSL::Certificate::decode(const std::string& encoding)
{
    return IceSSL::SecureTransport::Certificate::decode(encoding);
}
