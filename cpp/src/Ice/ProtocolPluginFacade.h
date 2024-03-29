//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PROTOCOL_PLUGIN_FACADE_H
#define ICE_PROTOCOL_PLUGIN_FACADE_H

#include "IceUtil/Config.h"
#include "ProtocolPluginFacadeF.h"
#include "Ice/CommunicatorF.h"
#include "EndpointFactoryF.h"
#include "Ice/InstanceF.h"
#include "EndpointIF.h"
#include "NetworkF.h"

#include <cstdint>

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
    class ICE_API ProtocolPluginFacade
    {
    public:
        ProtocolPluginFacade(const Ice::CommunicatorPtr&);
        virtual ~ProtocolPluginFacade();

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
        EndpointFactoryPtr getEndpointFactory(std::int16_t) const;

    private:
        friend ICE_API ProtocolPluginFacadePtr getProtocolPluginFacade(const Ice::CommunicatorPtr&);

        InstancePtr _instance;
        Ice::CommunicatorPtr _communicator;
    };
}

#endif
