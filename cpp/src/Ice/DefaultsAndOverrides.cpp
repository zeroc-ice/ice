// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/DefaultsAndOverrides.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(DefaultsAndOverrides* p) { return p; }

IceInternal::DefaultsAndOverrides::DefaultsAndOverrides(const PropertiesPtr& properties, const LoggerPtr& logger) :
    overrideTimeout(false),
    overrideTimeoutValue(-1),
    overrideConnectTimeout(false),
    overrideConnectTimeoutValue(-1),
    overrideCloseTimeout(false),
    overrideCloseTimeoutValue(-1),
    overrideCompress(false),
    overrideCompressValue(false),
    overrideSecure(false),
    overrideSecureValue(false)
{
    const_cast<string&>(defaultProtocol) = properties->getPropertyWithDefault("Ice.Default.Protocol", "tcp");

    const_cast<string&>(defaultHost) = properties->getProperty("Ice.Default.Host");

    string value;

#ifndef ICE_OS_WINRT
    value = properties->getProperty("Ice.Default.SourceAddress");
    if(!value.empty())
    {
        const_cast<Address&>(defaultSourceAddress) = getNumericAddress(value);
        if(!isAddressValid(defaultSourceAddress))
        {
            InitializationException ex(__FILE__, __LINE__);
            ex.reason = "invalid IP address set for Ice.Default.SourceAddress: `" + value + "'";
            throw ex;
        }
    }
#endif

    value = properties->getProperty("Ice.Override.Timeout");
    if(!value.empty())
    {
        const_cast<bool&>(overrideTimeout) = true;
        const_cast<Int&>(overrideTimeoutValue) = properties->getPropertyAsInt("Ice.Override.Timeout");
        if(overrideTimeoutValue < 1 && overrideTimeoutValue != -1)
        {
            const_cast<Int&>(overrideTimeoutValue) = -1;
            Warning out(logger);
            out << "invalid value for Ice.Override.Timeout `" << properties->getProperty("Ice.Override.Timeout")
                << "': defaulting to -1";
        }
    }

    value = properties->getProperty("Ice.Override.ConnectTimeout");
    if(!value.empty())
    {
        const_cast<bool&>(overrideConnectTimeout) = true;
        const_cast<Int&>(overrideConnectTimeoutValue) = properties->getPropertyAsInt("Ice.Override.ConnectTimeout");
        if(overrideConnectTimeoutValue < 1 && overrideConnectTimeoutValue != -1)
        {
            const_cast<Int&>(overrideConnectTimeoutValue) = -1;
            Warning out(logger);
            out << "invalid value for Ice.Override.ConnectTimeout `"
                << properties->getProperty("Ice.Override.ConnectTimeout") << "': defaulting to -1";
        }
    }

    value = properties->getProperty("Ice.Override.CloseTimeout");
    if(!value.empty())
    {
        const_cast<bool&>(overrideCloseTimeout) = true;
        const_cast<Int&>(overrideCloseTimeoutValue) = properties->getPropertyAsInt("Ice.Override.CloseTimeout");
        if(overrideCloseTimeoutValue < 1 && overrideCloseTimeoutValue != -1)
        {
            const_cast<Int&>(overrideCloseTimeoutValue) = -1;
            Warning out(logger);
            out << "invalid value for Ice.Override.CloseTimeout `"
                << properties->getProperty("Ice.Override.CloseTimeout") << "': defaulting to -1";
        }
    }

    value = properties->getProperty("Ice.Override.Compress");
    if(!value.empty())
    {
        const_cast<bool&>(overrideCompress) = true;
        const_cast<bool&>(overrideCompressValue) = properties->getPropertyAsInt("Ice.Override.Compress") > 0;
    }

    value = properties->getProperty("Ice.Override.Secure");
    if(!value.empty())
    {
        const_cast<bool&>(overrideSecure) = true;
        const_cast<bool&>(overrideSecureValue) = properties->getPropertyAsInt("Ice.Override.Secure") > 0;
    }

    const_cast<bool&>(defaultCollocationOptimization) =
        properties->getPropertyAsIntWithDefault("Ice.Default.CollocationOptimized", 1) > 0;

    value = properties->getPropertyWithDefault("Ice.Default.EndpointSelection", "Random");
    if(value == "Random")
    {
        defaultEndpointSelection = Random;
    }
    else if(value == "Ordered")
    {
        defaultEndpointSelection = Ordered;
    }
    else
    {
        EndpointSelectionTypeParseException ex(__FILE__, __LINE__);
        ex.str = "illegal value `" + value + "'; expected `Random' or `Ordered'";
        throw ex;
    }

    const_cast<int&>(defaultTimeout) =
        properties->getPropertyAsIntWithDefault("Ice.Default.Timeout", 60000);
    if(defaultTimeout < 1 && defaultTimeout != -1)
    {
        const_cast<Int&>(defaultTimeout) = 60000;
        Warning out(logger);
        out << "invalid value for Ice.Default.Timeout `" << properties->getProperty("Ice.Default.Timeout")
            << "': defaulting to 60000";
    }

    const_cast<int&>(defaultInvocationTimeout) =
        properties->getPropertyAsIntWithDefault("Ice.Default.InvocationTimeout", -1);
    if(defaultInvocationTimeout < 1 && defaultInvocationTimeout != -1 && defaultInvocationTimeout != -2)
    {
        const_cast<Int&>(defaultInvocationTimeout) = -1;
        Warning out(logger);
        out << "invalid value for Ice.Default.InvocationTimeout `"
            << properties->getProperty("Ice.Default.InvocationTimeout") << "': defaulting to -1";
    }

    const_cast<int&>(defaultLocatorCacheTimeout) =
        properties->getPropertyAsIntWithDefault("Ice.Default.LocatorCacheTimeout", -1);
    if(defaultLocatorCacheTimeout < -1)
    {
        const_cast<Int&>(defaultLocatorCacheTimeout) = -1;
        Warning out(logger);
        out << "invalid value for Ice.Default.LocatorCacheTimeout `"
            << properties->getProperty("Ice.Default.LocatorCacheTimeout") << "': defaulting to -1";
    }

    const_cast<bool&>(defaultPreferSecure) =
        properties->getPropertyAsIntWithDefault("Ice.Default.PreferSecure", 0) > 0;

    value = properties->getPropertyWithDefault("Ice.Default.EncodingVersion", encodingVersionToString(currentEncoding));
    defaultEncoding = stringToEncodingVersion(value);
    checkSupportedEncoding(defaultEncoding);

    bool slicedFormat = properties->getPropertyAsIntWithDefault("Ice.Default.SlicedFormat", 0) > 0;
    const_cast<FormatType&>(defaultFormat) = slicedFormat ? SlicedFormat : CompactFormat;
}
