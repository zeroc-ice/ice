// Copyright (c) ZeroC, Inc.

#include "DefaultsAndOverrides.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Properties.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::DefaultsAndOverrides::DefaultsAndOverrides(const PropertiesPtr& properties) : overrideCompress(nullopt)
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
                "invalid IP address set for Ice.Default.SourceAddress: '" + value + "'");
        }
    }

    value = properties->getIceProperty("Ice.Override.Compress");
    if (!value.empty())
    {
        const_cast<optional<bool>&>(overrideCompress) = properties->getIcePropertyAsInt("Ice.Override.Compress") > 0;
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

    const_cast<chrono::milliseconds&>(defaultInvocationTimeout) =
        chrono::milliseconds(properties->getIcePropertyAsInt("Ice.Default.InvocationTimeout"));
    if (defaultInvocationTimeout.count() < 1 && defaultInvocationTimeout.count() != -1)
    {
        throw InitializationException{
            __FILE__,
            __LINE__,
            "invalid value for Ice.Default.InvocationTimeout: " + to_string(defaultInvocationTimeout.count())};
    }

    const_cast<chrono::seconds&>(defaultLocatorCacheTimeout) =
        chrono::seconds(properties->getIcePropertyAsInt("Ice.Default.LocatorCacheTimeout"));
    if (defaultLocatorCacheTimeout.count() < -1)
    {
        throw InitializationException{
            __FILE__,
            __LINE__,
            "invalid value for Ice.Default.LocatorCacheTimeout: " + to_string(defaultLocatorCacheTimeout.count())};
    }

    value = properties->getIceProperty("Ice.Default.EncodingVersion");
    defaultEncoding = stringToEncodingVersion(value);
    checkSupportedEncoding(defaultEncoding);

    bool slicedFormat = properties->getIcePropertyAsInt("Ice.Default.SlicedFormat") > 0;
    const_cast<FormatType&>(defaultFormat) = slicedFormat ? FormatType::SlicedFormat : FormatType::CompactFormat;
}
