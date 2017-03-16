// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
#include <Ice/RegisterPlugins.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

#ifndef ICE_CPP11_MAPPING
CertificateVerifier::~CertificateVerifier()
{
    // Out of line to avoid weak vtable
}

PasswordPrompt::~PasswordPrompt()
{
    // Out of line to avoid weak vtable
}
#endif


NativeConnectionInfo::~NativeConnectionInfo()
{
    // Out of line to avoid weak vtable
}

IceSSL::Plugin::~Plugin()
{
    // Out of line to avoid weak vtable
}

//
// Plugin implementation.
//
PluginI::PluginI(const Ice::CommunicatorPtr& com, const SSLEnginePtr& engine) :
    _engine(engine)
{
    //
    // Register the endpoint factory. We have to do this now, rather
    // than in initialize, because the communicator may need to
    // interpret proxies before the plug-in is fully initialized.
    //
    IceInternal::ProtocolPluginFacadePtr pluginFacade = IceInternal::getProtocolPluginFacade(com);

    // SSL based on TCP
    IceInternal::EndpointFactoryPtr tcp = pluginFacade->getEndpointFactory(TCPEndpointType);
    if(tcp)
    {
        InstancePtr instance = new Instance(_engine, SSLEndpointType, "ssl");
        pluginFacade->addEndpointFactory(new EndpointFactoryI(instance, tcp->clone(instance, 0)));
    }

    // SSL based on Bluetooth
    IceInternal::EndpointFactoryPtr bluetooth = pluginFacade->getEndpointFactory(BTEndpointType);
    if(bluetooth)
    {
        InstancePtr instance = new Instance(_engine, BTSEndpointType, "bts");
        pluginFacade->addEndpointFactory(new EndpointFactoryI(instance, bluetooth->clone(instance, 0)));
    }

    // SSL based on iAP
    IceInternal::EndpointFactoryPtr iap = pluginFacade->getEndpointFactory(iAPEndpointType);
    if(iap)
    {
        InstancePtr instance = new Instance(_engine, iAPSEndpointType, "iaps");
        pluginFacade->addEndpointFactory(new EndpointFactoryI(instance, iap->clone(instance, 0)));
    }
}

void
PluginI::initialize()
{
    _engine->initialize();
}

void
PluginI::destroy()
{
    _engine->destroy();
    _engine = 0;
}

#ifdef ICE_CPP11_MAPPING
void
PluginI::setCertificateVerifier(std::function<bool(const std::shared_ptr<NativeConnectionInfo>&)> verifier)
{
    if(verifier)
    {
        _engine->setCertificateVerifier(make_shared<CertificateVerifier>(std::move(verifier)));
    }
    else
    {
        _engine->setCertificateVerifier(nullptr);
    }
}
#else
void
PluginI::setCertificateVerifier(const CertificateVerifierPtr& verifier)
{
    _engine->setCertificateVerifier(verifier);
}
#endif

#ifdef ICE_CPP11_MAPPING
void
PluginI::setPasswordPrompt(std::function<std::string()> prompt)
{
     if(prompt)
     {
         _engine->setPasswordPrompt(make_shared<PasswordPrompt>(std::move(prompt)));
     }
     else
     {
         _engine->setPasswordPrompt(nullptr);
     }
}
#else
void
PluginI::setPasswordPrompt(const PasswordPromptPtr& prompt)
{
    _engine->setPasswordPrompt(prompt);
}
#endif

extern "C"
{

ICESSL_API Ice::Plugin*
createIceSSL(const CommunicatorPtr&, const string&, const StringSeq&);

}

namespace Ice
{

ICESSL_API void
registerIceSSL(bool loadOnInitialize)
{
    Ice::registerPluginFactory("IceSSL", createIceSSL, loadOnInitialize);
}

}

//
// Objective-C function to allow Objective-C programs to register plugin.
//
extern "C" ICESSL_API void
ICEregisterIceSSL(bool loadOnInitialize)
{
    Ice::registerIceSSL(loadOnInitialize);
}

