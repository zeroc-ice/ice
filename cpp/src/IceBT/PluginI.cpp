// Copyright (c) ZeroC, Inc.

#include "PluginI.h"
#include "../Ice/ProtocolInstance.h"
#include "../Ice/ProtocolPluginFacade.h"
#include "EndpointI.h"
#include "Engine.h"
#include "Ice/LocalExceptions.h"
#include "Instance.h"
#include "Util.h"

using namespace std;
using namespace Ice;
using namespace IceBT;

//
// Plug-in factory function.
//
extern "C"
{
    ICEBT_API Ice::Plugin*
    createIceBT(const CommunicatorPtr& communicator, const string& /*name*/, const StringSeq& /*args*/)
    {
        return new PluginI(communicator);
    }
}

namespace Ice
{
    ICEBT_API PluginFactory btPluginFactory() { return {"IceBT", createIceBT}; }
}

//
// Plugin implementation.
//
IceBT::PluginI::PluginI(const Ice::CommunicatorPtr& com) : _engine(new Engine(com))
{
    IceInternal::ProtocolPluginFacadePtr f = IceInternal::getProtocolPluginFacade(com);

    //
    // Register the endpoint factory. We have to do this now, rather
    // than in initialize, because the communicator may need to
    // interpret proxies before the plug-in is fully initialized.
    //
    InstancePtr bt = make_shared<Instance>(_engine, BTEndpointType, "bt");
    f->addEndpointFactory(make_shared<EndpointFactoryI>(bt));

    InstancePtr bts = make_shared<Instance>(_engine, BTSEndpointType, "bts");
    f->addEndpointFactory(make_shared<IceInternal::UnderlyingEndpointFactory>(bts, SSLEndpointType, BTEndpointType));
}

void
IceBT::PluginI::initialize()
{
    _engine->initialize();
}

void
IceBT::PluginI::destroy()
{
    _engine->destroy();
    _engine = nullptr;
}

void
IceBT::PluginI::startDiscovery(const string& address, function<void(const string&, const PropertyMap&)> cb)
{
    _engine->startDiscovery(address, cb);
}

void
IceBT::PluginI::stopDiscovery(const string& address)
{
    _engine->stopDiscovery(address);
}

IceBT::DeviceMap
IceBT::PluginI::getDevices() const
{
    return _engine->getDevices();
}
