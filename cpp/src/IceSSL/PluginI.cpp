// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/PluginI.h>
#include <IceSSL/Instance.h>
#include <IceSSL/TransceiverI.h>
#include <IceSSL/SSLEngine.h>
#include <IceSSL/EndpointI.h>
#include <IceSSL/EndpointInfo.h>

#include <Ice/ProtocolPluginFacade.h>
#include <Ice/LocalException.h>
#include <Ice/Object.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

//
// Plug-in factory function.
//
extern "C"
{

ICE_DECLSPEC_EXPORT Ice::Plugin*
createIceSSL(const CommunicatorPtr& communicator, const string& /*name*/, const StringSeq& /*args*/)
{
    return new PluginI(communicator);
}

}

//
// Plugin implementation.
//
IceSSL::PluginI::PluginI(const Ice::CommunicatorPtr& communicator)
{
#ifdef ICE_USE_OPENSSL
    _engine = new OpenSSLEngine(communicator);
#else
    _engine = new SecureTransportEngine(communicator);
#endif
    
    IceInternal::ProtocolPluginFacadePtr facade = IceInternal::getProtocolPluginFacade(communicator);
    
    //
    // Register the endpoint factory. We have to do this now, rather than
    // in initialize, because the communicator may need to interpret
    // proxies before the plug-in is fully initialized.
    //
    facade->addEndpointFactory(new EndpointFactoryI(new Instance(_engine, EndpointType, "ssl")));
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
