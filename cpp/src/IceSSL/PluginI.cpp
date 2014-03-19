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
    PluginI* plugin = new PluginI(communicator);
    return plugin;
}

}

//
// Plugin implementation.
//
IceSSL::PluginI::PluginI(const Ice::CommunicatorPtr& communicator)
{
    _sharedInstance = new SharedInstance(communicator);

    IceInternal::ProtocolPluginFacadePtr facade = IceInternal::getProtocolPluginFacade(communicator);
    
    //
    // Register the endpoint factory. We have to do this now, rather than
    // in initialize, because the communicator may need to interpret
    // proxies before the plug-in is fully initialized.
    //
    facade->addEndpointFactory(new EndpointFactoryI(new Instance(_sharedInstance, EndpointType, "ssl")));
}

void
IceSSL::PluginI::initialize()
{
    _sharedInstance->initialize();
}

void
IceSSL::PluginI::destroy()
{
    _sharedInstance->destroy();
    _sharedInstance = 0;
}

void
IceSSL::PluginI::setContext(SSL_CTX* context)
{
    _sharedInstance->context(context);
}

SSL_CTX*
IceSSL::PluginI::getContext()
{
    return _sharedInstance->context();
}

void
IceSSL::PluginI::setCertificateVerifier(const CertificateVerifierPtr& verifier)
{
    _sharedInstance->setCertificateVerifier(verifier);
}

void
IceSSL::PluginI::setPasswordPrompt(const PasswordPromptPtr& prompt)
{
    _sharedInstance->setPasswordPrompt(prompt);
}
