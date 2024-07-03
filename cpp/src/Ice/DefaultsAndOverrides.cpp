//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "DefaultsAndOverrides.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"
#include "Ice/Properties.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::DefaultsAndOverrides::DefaultsAndOverrides(const PropertiesPtr& properties, const LoggerPtr& logger)
    : overrideCompress(nullopt),
      overrideSecure(nullopt)
{
    const_cast<string&>(defaultProtocol) = properties->getIceProperty("Ice.Default.Protocol");

    const_cast<string&>(defaultHost) = properties->getIceProperty("Ice.Default.Host");

    string value;

    value = properties->getIceProperty("Ice.Default.SourceAddress");
    if (!value.empty())
    {
        const_cast<Address&>(defaultSourceAddress) = getNumericAddress(value);
        if (!isAddressValid(defaultSourceAddress))
        {
            throw InitializationException(
                __FILE__,
                __LINE__,
                "invalid IP address set for Ice.Default.SourceAddress: `" + value + "'");
        }
    }

    value = properties->getIceProperty("Ice.Override.Compress");
    if (!value.empty())
    {
        const_cast<optional<bool>&>(overrideCompress) = properties->getIcePropertyAsInt("Ice.Override.Compress") > 0;
    }

    value = properties->getIceProperty("Ice.Override.Secure");
    if (!value.empty())
    {
        const_cast<optional<bool>&>(overrideSecure) = properties->getIcePropertyAsInt("Ice.Override.Secure") > 0;
    }

    const_cast<bool&>(defaultCollocationOptimization) =
        properties->getIcePropertyAsInt("Ice.Default.CollocationOptimized") > 0;

    value = properties->getIceProperty("Ice.Default.EndpointSelection");
    if (value == "Random")
    {
        defaultEndpointSelection = EndpointSelectionType::Random;
    }
    else if (value == "Ordered")
    {
        defaultEndpointSelection = EndpointSelectionType::Ordered;
    }
    else
    {
        throw ParseException(__FILE__, __LINE__, "illegal value '" + value + "'; expected 'Random' or 'Ordered'");
    }

    const_cast<int&>(defaultInvocationTimeout) = properties->getIcePropertyAsInt("Ice.Default.InvocationTimeout");
    if (defaultInvocationTimeout < 1 && defaultInvocationTimeout != -1)
    {
        const_cast<int32_t&>(defaultInvocationTimeout) = -1;
        Warning out(logger);
        out << "invalid value for Ice.Default.InvocationTimeout `"
            << properties->getIceProperty("Ice.Default.InvocationTimeout") << "': defaulting to -1";
    }

    const_cast<int&>(defaultLocatorCacheTimeout) = properties->getIcePropertyAsInt("Ice.Default.LocatorCacheTimeout");
    if (defaultLocatorCacheTimeout < -1)
    {
        const_cast<int32_t&>(defaultLocatorCacheTimeout) = -1;
        Warning out(logger);
        out << "invalid value for Ice.Default.LocatorCacheTimeout `"
            << properties->getIceProperty("Ice.Default.LocatorCacheTimeout") << "': defaulting to -1";
    }

    const_cast<bool&>(defaultPreferSecure) = properties->getIcePropertyAsInt("Ice.Default.PreferSecure") > 0;

    value = properties->getPropertyWithDefault("Ice.Default.EncodingVersion", encodingVersionToString(currentEncoding));
    defaultEncoding = stringToEncodingVersion(value);
    checkSupportedEncoding(defaultEncoding);

    bool slicedFormat = properties->getIcePropertyAsInt("Ice.Default.SlicedFormat") > 0;
    const_cast<FormatType&>(defaultFormat) = slicedFormat ? FormatType::SlicedFormat : FormatType::CompactFormat;
}
