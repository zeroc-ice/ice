
// Copyright (c) ZeroC, Inc.

#ifndef ICE_DEFAULTS_AND_OVERRIDES_H
#define ICE_DEFAULTS_AND_OVERRIDES_H

#include "DefaultsAndOverridesF.h"
#include "Ice/Endpoint.h"
#include "Ice/EndpointSelectionType.h"
#include "Ice/EndpointTypes.h"
#include "Ice/Format.h"
#include "Ice/PropertiesF.h"
#include "Network.h"

#include <chrono>

namespace IceInternal
{
    class DefaultsAndOverrides
    {
    public:
        DefaultsAndOverrides(const Ice::PropertiesPtr&);

        std::string defaultHost;
        Address defaultSourceAddress;
        std::string defaultProtocol;
        bool defaultCollocationOptimization;
        Ice::EndpointSelectionType defaultEndpointSelection;
        std::chrono::milliseconds defaultInvocationTimeout;
        std::chrono::seconds defaultLocatorCacheTimeout;
        Ice::EncodingVersion defaultEncoding;
        Ice::FormatType defaultFormat;

        std::optional<bool> overrideCompress;
    };
}

#endif
