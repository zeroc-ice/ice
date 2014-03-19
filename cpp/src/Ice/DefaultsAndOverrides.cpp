// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/Properties.h>
#include <Ice/Network.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(DefaultsAndOverrides* p) { return p; }

IceInternal::DefaultsAndOverrides::DefaultsAndOverrides(const PropertiesPtr& properties) :
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
    
    value = properties->getProperty("Ice.Override.Timeout");
    if(!value.empty())
    {
        const_cast<bool&>(overrideTimeout) = true;
        const_cast<Int&>(overrideTimeoutValue) = properties->getPropertyAsInt("Ice.Override.Timeout");
    }

    value = properties->getProperty("Ice.Override.ConnectTimeout");
    if(!value.empty())
    {
        const_cast<bool&>(overrideConnectTimeout) = true;
        const_cast<Int&>(overrideConnectTimeoutValue) = properties->getPropertyAsInt("Ice.Override.ConnectTimeout");
    }

    value = properties->getProperty("Ice.Override.CloseTimeout");
    if(!value.empty())
    {
        const_cast<bool&>(overrideCloseTimeout) = true;
        const_cast<Int&>(overrideCloseTimeoutValue) = properties->getPropertyAsInt("Ice.Override.CloseTimeout");
    }

    value = properties->getProperty("Ice.Override.Compress");
    if(!value.empty())
    {
        const_cast<bool&>(overrideCompress) = true;
        const_cast<bool&>(overrideCompressValue) = properties->getPropertyAsInt("Ice.Override.Compress");
    }

    value = properties->getProperty("Ice.Override.Secure");
    if(!value.empty())
    {
        const_cast<bool&>(overrideSecure) = true;
        const_cast<bool&>(overrideSecureValue) = properties->getPropertyAsInt("Ice.Override.Secure");
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

    const_cast<int&>(defaultLocatorCacheTimeout) = 
        properties->getPropertyAsIntWithDefault("Ice.Default.LocatorCacheTimeout", -1);

    const_cast<bool&>(defaultPreferSecure) =
        properties->getPropertyAsIntWithDefault("Ice.Default.PreferSecure", 0) > 0;

    value = properties->getPropertyWithDefault("Ice.Default.EncodingVersion", encodingVersionToString(currentEncoding));
    defaultEncoding = stringToEncodingVersion(value);
    checkSupportedEncoding(defaultEncoding);

    bool slicedFormat = properties->getPropertyAsIntWithDefault("Ice.Default.SlicedFormat", 0) > 0;
    const_cast<FormatType&>(defaultFormat) = slicedFormat ? SlicedFormat : CompactFormat;
}
