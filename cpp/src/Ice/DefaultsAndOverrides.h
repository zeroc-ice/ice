
// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <IceUtil/Shared.h>
#include <Ice/DefaultsAndOverridesF.h>
#include <Ice/PropertiesF.h>
#include <Ice/Endpoint.h>
#include <Ice/Format.h>

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
    Ice::EncodingVersion defaultEncoding;
    Ice::FormatType defaultFormat;

    bool overrideTimeout;
    Ice::Int overrideTimeoutValue;
    bool overrideConnectTimeout;
    Ice::Int overrideConnectTimeoutValue;
    bool overrideCloseTimeout;
    Ice::Int overrideCloseTimeoutValue;
    bool overrideCompress;
    bool overrideCompressValue;
    bool overrideSecure;
    bool overrideSecureValue;
};

}
