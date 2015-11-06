// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceBT/PluginI.h>
#include <IceBT/EndpointI.h>
#include <IceBT/Engine.h>
#include <IceBT/Instance.h>
#include <IceBT/Util.h>

#include <Ice/LocalException.h>
#include <Ice/ProtocolPluginFacade.h>
#include <Ice/ProtocolInstance.h>

using namespace std;
using namespace Ice;
using namespace IceBT;

void
IceBT::BluetoothException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nbluetooth exception: `" << reason << "'";
}

//
// Plug-in factory function.
//
extern "C"
{

ICE_BT_API Ice::Plugin*
createIceBT(const CommunicatorPtr& communicator, const string& /*name*/, const StringSeq& /*args*/)
{
    return new PluginI(communicator);
}

}

//
// Plugin implementation.
//
IceBT::PluginI::PluginI(const Ice::CommunicatorPtr& com) :
    _engine(new Engine(com))
{
    //
    // Register the endpoint factory. We have to do this now, rather
    // than in initialize, because the communicator may need to
    // interpret proxies before the plug-in is fully initialized.
    //
    IceInternal::EndpointFactoryPtr btFactory = new EndpointFactoryI(new Instance(_engine, EndpointType, "bt"));
    IceInternal::getProtocolPluginFacade(com)->addEndpointFactory(btFactory);
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
}
