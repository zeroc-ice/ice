// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <IceSSL/PluginI.h>
#include <IceSSL/OpenSSL.h>
#include <IceSSL/OpenSSLEngine.h>

#include <Ice/Initialize.h>

using namespace std;

namespace
{

class PluginI : public IceSSL::PluginI,
                public IceSSL::OpenSSL::Plugin
{
public:

    PluginI(const Ice::CommunicatorPtr&);

    virtual Ice::Long getOpenSSLVersion() const;
    virtual IceSSL::CertificatePtr create(x509_st*) const;
    virtual IceSSL::CertificatePtr load(const std::string&) const;
    virtual IceSSL::CertificatePtr decode(const std::string&) const;
    virtual void setContext(SSL_CTX*);
    virtual SSL_CTX* getContext();
};

} // anonymous namespace end

//
// Plugin implementation.
//
PluginI::PluginI(const Ice::CommunicatorPtr& com) :
    IceSSL::PluginI(com, new IceSSL::OpenSSL::SSLEngine(com))
{
}

Ice::Long
PluginI::getOpenSSLVersion() const
{
    return SSLeay();
}

IceSSL::CertificatePtr
PluginI::create(x509_st* cert) const
{
    return IceSSL::OpenSSL::Certificate::create(cert);
}

IceSSL::CertificatePtr
PluginI::load(const std::string& file) const
{
    return IceSSL::OpenSSL::Certificate::load(file);
}

IceSSL::CertificatePtr
PluginI::decode(const std::string& encoding) const
{
    return IceSSL::OpenSSL::Certificate::load(encoding);
}

void
PluginI::setContext(SSL_CTX* context)
{
    IceSSL::OpenSSL::SSLEngine* engine = dynamic_cast<IceSSL::OpenSSL::SSLEngine*>(_engine.get());
    assert(engine);
    engine->context(context);
}

SSL_CTX*
PluginI::getContext()
{
    IceSSL::OpenSSL::SSLEngine* engine = dynamic_cast<IceSSL::OpenSSL::SSLEngine*>(_engine.get());
    assert(engine);
    return engine->context();
}

#ifdef _WIN32
//
// Plug-in factory function.
//
extern "C" ICESSL_OPENSSL_API Ice::Plugin*
createIceSSLOpenSSL(const Ice::CommunicatorPtr& communicator, const string& /*name*/, const Ice::StringSeq& /*args*/)
{
    return new PluginI(communicator);
}

namespace Ice
{

ICESSL_OPENSSL_API void
registerIceSSLOpenSSL(bool loadOnInitialize)
{
    Ice::registerPluginFactory("IceSSL", createIceSSLOpenSSL, loadOnInitialize);
}

}

#else

extern "C" ICESSL_API Ice::Plugin*
createIceSSL(const Ice::CommunicatorPtr& communicator, const string& /*name*/, const Ice::StringSeq& /*args*/)
{
    return new PluginI(communicator);
}

//
// The following functions are defined only when OpenSSL is the default
// implementation. In Windows the default implementation is always
// SChannel.
//
IceSSL::CertificatePtr
IceSSL::Certificate::load(const std::string& file)
{
    return IceSSL::OpenSSL::Certificate::load(file);
}

IceSSL::CertificatePtr
IceSSL::Certificate::decode(const std::string& encoding)
{
    return IceSSL::OpenSSL::Certificate::decode(encoding);
}

#endif
