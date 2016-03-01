// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PROTOCOL_PLUGIN_FACADE_H
#define ICE_PROTOCOL_PLUGIN_FACADE_H

#include <IceUtil/Config.h>
#include <IceUtil/Shared.h>
#include <Ice/ProtocolPluginFacadeF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/EndpointFactoryF.h>
#include <Ice/InstanceF.h>
#include <Ice/EndpointIF.h>
#include <Ice/NetworkF.h>

namespace IceInternal
{

//
// Global function to obtain a ProtocolPluginFacade given a Communicator
// instance.
//
ICE_API ProtocolPluginFacadePtr getProtocolPluginFacade(const Ice::CommunicatorPtr&);

//
// ProtocolPluginFacade wraps the internal operations that protocol
// plug-ins may need.
//
class ICE_API ProtocolPluginFacade : public ::IceUtil::Shared
{
public:

    //
    // Get the Communicator instance with which this facade is
    // associated.
    //
    Ice::CommunicatorPtr getCommunicator() const;

    //
    // Register an EndpointFactory.
    //
    void addEndpointFactory(const EndpointFactoryPtr&) const;

    //
    // Get an EndpointFactory.
    //
    EndpointFactoryPtr getEndpointFactory(Ice::Short) const;

private:

    ProtocolPluginFacade(const Ice::CommunicatorPtr&);

    friend ICE_API ProtocolPluginFacadePtr getProtocolPluginFacade(const Ice::CommunicatorPtr&);

    InstancePtr _instance;
    Ice::CommunicatorPtr _communicator;
};

}

#endif
