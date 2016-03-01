// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/PluginI.h>
#include <IceSSL/Instance.h>
#include <IceSSL/SSLEngine.h>
#include <IceSSL/EndpointI.h>

#include <Ice/ProtocolPluginFacade.h>
#include <Ice/ProtocolInstance.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

//
// Plug-in factory function.
//
extern "C"
{

ICE_SSL_API Ice::Plugin*
createIceSSL(const CommunicatorPtr& communicator, const string& /*name*/, const StringSeq& /*args*/)
{
    return new PluginI(communicator);
}

}

namespace Ice
{

ICE_SSL_API void
registerIceSSL(bool loadOnInitialize)
{
    Ice::registerPluginFactory("IceSSL", createIceSSL, loadOnInitialize);
}

}

//
// Plugin implementation.
//
IceSSL::PluginI::PluginI(const Ice::CommunicatorPtr& com)
{
#if defined(ICE_USE_SECURE_TRANSPORT)
    _engine = new SecureTransportEngine(com);
#elif defined(ICE_USE_SCHANNEL)
    _engine = new SChannelEngine(com);
#else
    _engine = new OpenSSLEngine(com);
#endif

    //
    // Register the endpoint factory. We have to do this now, rather
    // than in initialize, because the communicator may need to
    // interpret proxies before the plug-in is fully initialized.
    //
    IceInternal::EndpointFactoryPtr sslFactory = new EndpointFactoryI(new Instance(_engine, EndpointType, "ssl"));
    IceInternal::getProtocolPluginFacade(com)->addEndpointFactory(sslFactory);
}

void
IceSSL::PluginI::initialize()
{
    _engine->initialize();
}

void
IceSSL::PluginI::destroy()
{
    _engine->destroy();
    _engine = 0;
}

void
IceSSL::PluginI::setCertificateVerifier(const CertificateVerifierPtr& verifier)
{
    _engine->setCertificateVerifier(verifier);
}

void
IceSSL::PluginI::setPasswordPrompt(const PasswordPromptPtr& prompt)
{
    _engine->setPasswordPrompt(prompt);
}

#ifdef ICE_USE_OPENSSL
void
IceSSL::PluginI::setContext(SSL_CTX* context)
{
    _engine->context(context);
}

SSL_CTX*
IceSSL::PluginI::getContext()
{
    return _engine->context();
}
#endif
