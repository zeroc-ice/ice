// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_DEFAULTS_AND_OVERRIDES_H
#define ICE_DEFAULTS_AND_OVERRIDES_H

#include <IceUtil/Shared.h>
#include <Ice/DefaultsAndOverridesF.h>
#include <Ice/PropertiesF.h>
#include <Ice/Endpoint.h>

namespace IceInternal
{

class DefaultsAndOverrides : public ::IceUtil::Shared
{
public:

    DefaultsAndOverrides(const ::Ice::PropertiesPtr&);

    std::string defaultHost;
    std::string defaultProtocol;
    bool defaultCollocationOptimization;
    Ice::EndpointSelectionType defaultEndpointSelection;
    int defaultLocatorCacheTimeout;
    bool defaultPreferSecure;

    bool overrideTimeout;
    Ice::Int overrideTimeoutValue;
    bool overrideConnectTimeout;
    Ice::Int overrideConnectTimeoutValue;
    bool overrideCompress;
    bool overrideCompressValue;
    bool overrideSecure;
    bool overrideSecureValue;
};

}

#endif
