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

namespace
{
    const char* const btPluginName = "IceBT";
}

//
// Plug-in factory function.
//
extern "C"
{
    ICEBT_API Ice::Plugin*
    createIceBT(const CommunicatorPtr& communicator, const string& name, const StringSeq& /*args*/)
    {
        string pluginName{btPluginName};

        if (name != pluginName)
        {
            throw PluginInitializationException{
                __FILE__,
                __LINE__,
                "the Bluetooth plug-in must be named '" + pluginName + "'"};
        }

        return new PluginI(communicator);
    }
}

PluginFactory
IceBT::btPluginFactory()
{
    return {btPluginName, createIceBT};
}

//
// Plugin implementation.
//
IceBT::PluginI::PluginI(const Ice::CommunicatorPtr& com) : _engine(new Engine(com))
{
    IceInternal::ProtocolPluginFacade facade{com};

    //
    // Register the endpoint factory. We have to do this now, rather
    // than in initialize, because the communicator may need to
    // interpret proxies before the plug-in is fully initialized.
    //
    InstancePtr bt = make_shared<Instance>(_engine, BTEndpointType, "bt");
    facade.addEndpointFactory(make_shared<EndpointFactoryI>(bt));

    InstancePtr bts = make_shared<Instance>(_engine, BTSEndpointType, "bts");
    facade.addEndpointFactory(
        make_shared<IceInternal::UnderlyingEndpointFactory>(bts, SSLEndpointType, BTEndpointType));
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
