
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_DEFAULTS_AND_OVERRIDES_H
#define ICE_DEFAULTS_AND_OVERRIDES_H

#include <Ice/DefaultsAndOverridesF.h>
#include <Ice/PropertiesF.h>
#include <Ice/Endpoint.h>
#include <Ice/EndpointTypes.h>
#include <Ice/Format.h>
#include <Ice/Network.h>

namespace IceInternal
{

    class DefaultsAndOverrides
    {
    public:
        DefaultsAndOverrides(const ::Ice::PropertiesPtr&, const ::Ice::LoggerPtr&);

        std::string defaultHost;
        Address defaultSourceAddress;
        std::string defaultProtocol;
        bool defaultCollocationOptimization;
        Ice::EndpointSelectionType defaultEndpointSelection;
        int defaultTimeout;
        int defaultInvocationTimeout;
        int defaultLocatorCacheTimeout;
        bool defaultPreferSecure;
        Ice::EncodingVersion defaultEncoding;
        Ice::FormatType defaultFormat;

        bool overrideTimeout;
        std::int32_t overrideTimeoutValue;
        bool overrideConnectTimeout;
        std::int32_t overrideConnectTimeoutValue;
        bool overrideCloseTimeout;
        std::int32_t overrideCloseTimeoutValue;
        bool overrideCompress;
        bool overrideCompressValue;
        bool overrideSecure;
        bool overrideSecureValue;
    };

}

#endif
