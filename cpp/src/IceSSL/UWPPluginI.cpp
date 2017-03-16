// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/PluginI.h>
#include <IceSSL/UWPEngine.h>

#include <Ice/Initialize.h>

using namespace std;

namespace
{

class PluginI : public IceSSL::PluginI
{
public:

    PluginI(const Ice::CommunicatorPtr&);
    //
    // Construct a certificate using a native certificate.
    //
    virtual string getEngineName() const;
    virtual Ice::Long getEngineVersion() const;
    virtual IceSSL::CertificatePtr create(Windows::Security::Cryptography::Certificates::Certificate^) const;
    virtual IceSSL::CertificatePtr load(const std::string&) const;
    virtual IceSSL::CertificatePtr decode(const std::string&) const;
};

} // anonymous namespace end

//
// Plugin implementation.
//
PluginI::PluginI(const Ice::CommunicatorPtr& com) :
    IceSSL::PluginI(com, new IceSSL::UWP::SSLEngine(com))
{
}

IceSSL::CertificatePtr
PluginI::create(Windows::Security::Cryptography::Certificates::Certificate^ cert) const
{
    return IceSSL::UWP::Certificate::create(cert);
}

IceSSL::CertificatePtr
PluginI::load(const std::string& file) const
{
    return IceSSL::UWP::Certificate::load(file);
}

IceSSL::CertificatePtr
PluginI::decode(const std::string& encoding) const
{
    return IceSSL::UWP::Certificate::decode(encoding);
}

string
PluginI::getEngineName() const
{
    return "UWPEngine";
}

Ice::Long
PluginI::getEngineVersion() const
{
    return 0;
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
    return IceSSL::UWP::Certificate::load(file);
}

IceSSL::CertificatePtr
IceSSL::Certificate::decode(const std::string& encoding)
{
    return IceSSL::UWP::Certificate::decode(encoding);
}