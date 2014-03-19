// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceWS/PluginI.h>
#include <IceWS/Instance.h>
#include <IceWS/EndpointI.h>
#include <IceWS/EndpointInfo.h>
#include <Ice/ProtocolPluginFacade.h>

using namespace std;
using namespace Ice;
using namespace IceWS;

//
// Plug-in factory function.
//
extern "C"
{

ICE_DECLSPEC_EXPORT Ice::Plugin*
createIceWS(const CommunicatorPtr& communicator, const string& /*name*/, const StringSeq& /*args*/)
{
    return new PluginI(communicator);
}

}

//
// Plugin implementation.
//
IceWS::PluginI::PluginI(const Ice::CommunicatorPtr& communicator)
{
    IceInternal::ProtocolPluginFacadePtr facade = getProtocolPluginFacade(communicator);
    IceInternal::EndpointFactoryPtr tcpFactory = facade->getEndpointFactory(TCPEndpointType);
    if(tcpFactory)
    {
        InstancePtr tcpInstance = new Instance(communicator, WSEndpointType, "ws");
        facade->addEndpointFactory(new EndpointFactoryI(tcpInstance, tcpFactory->clone(tcpInstance)));
    }
    IceInternal::EndpointFactoryPtr sslFactory = facade->getEndpointFactory(2); // 2 = SSLEndpointType
    if(sslFactory)
    {
        InstancePtr sslInstance = new Instance(communicator, WSSEndpointType, "wss");
        facade->addEndpointFactory(new EndpointFactoryI(sslInstance, sslFactory->clone(sslInstance)));
    }
}

void
IceWS::PluginI::initialize()
{
}

void
IceWS::PluginI::destroy()
{
}
