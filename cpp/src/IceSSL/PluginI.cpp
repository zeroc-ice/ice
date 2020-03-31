//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
    InstancePtr instance = new Instance(_engine, SSLEndpointType, "ssl"); // SSL based on TCP
    IceInternal::getProtocolPluginFacade(com)->addEndpointFactory(new EndpointFactoryI(instance, TCPEndpointType));
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

void
PluginI::setCertificateVerifier(std::function<bool(const std::shared_ptr<IceSSL::ConnectionInfo>&)> verifier)
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
