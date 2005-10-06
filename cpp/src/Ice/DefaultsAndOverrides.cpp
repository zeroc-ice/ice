// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/DefaultsAndOverrides.h>
#include <Ice/Properties.h>
#include <Ice/Network.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(DefaultsAndOverrides* p) { p->__incRef(); }
void IceInternal::decRef(DefaultsAndOverrides* p) { p->__decRef(); }

IceInternal::DefaultsAndOverrides::DefaultsAndOverrides(const PropertiesPtr& properties) :
    overrideTimeout(false),
    overrideTimeoutValue(-1),
    overrideConnectTimeout(false),
    overrideConnectTimeoutValue(-1),
    overrideCompress(false),
    overrideCompressValue(false)
{
    const_cast<string&>(defaultProtocol) = properties->getPropertyWithDefault("Ice.Default.Protocol", "tcp");

    const_cast<string&>(defaultHost) = properties->getProperty("Ice.Default.Host");

    const_cast<string&>(defaultRouter) = properties->getProperty("Ice.Default.Router");

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

    value = properties->getProperty("Ice.Override.Compress");
    if(!value.empty())
    {
	const_cast<bool&>(overrideCompress) = true;
	const_cast<bool&>(overrideCompressValue) = properties->getPropertyAsInt("Ice.Override.Compress");
    }

    const_cast<string&>(defaultLocator) = properties->getProperty("Ice.Default.Locator");

    const_cast<bool&>(defaultCollocationOptimization) =
	properties->getPropertyAsIntWithDefault("Ice.Default.CollocationOptimization", 1) > 0;
}
