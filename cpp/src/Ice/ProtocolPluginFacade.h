// Copyright (c) ZeroC, Inc.

#ifndef ICE_PROTOCOL_PLUGIN_FACADE_H
#define ICE_PROTOCOL_PLUGIN_FACADE_H

#include "EndpointFactoryF.h"
#include "EndpointIF.h"
#include "Ice/CommunicatorF.h"
#include "Ice/Config.h"
#include "Ice/InstanceF.h"
#include "NetworkF.h"

#include <cstdint>

namespace IceInternal
{
    /// Provides limited access to the internal endpoint factory manager. Used by transport plug-ins.
    class ICE_API ProtocolPluginFacade final
    {
    public:
        /// Constructs a plug-in facade.
        ProtocolPluginFacade(const Ice::CommunicatorPtr&);

        /// Registers an EndpointFactory.
        void addEndpointFactory(const EndpointFactoryPtr&) const;

        /// Gets the EndpointFactory for a given Endpoint type.
        [[nodiscard]] EndpointFactoryPtr getEndpointFactory(std::int16_t) const;

    private:
        InstancePtr _instance;
    };
}

#endif
