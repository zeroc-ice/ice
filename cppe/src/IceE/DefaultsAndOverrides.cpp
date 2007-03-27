// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/DefaultsAndOverrides.h>
#include <IceE/Properties.h>
#include <IceE/Network.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(DefaultsAndOverrides* p) { return p; }

IceInternal::DefaultsAndOverrides::DefaultsAndOverrides(const PropertiesPtr& properties) :
    overrideTimeout(false),
    overrideTimeoutValue(-1),
    overrideConnectTimeout(false),
    overrideConnectTimeoutValue(-1)
{
    const_cast<string&>(defaultHost) = properties->getProperty("Ice.Default.Host");

#ifdef ICEE_HAS_ROUTER
    const_cast<string&>(defaultRouter) = properties->getProperty("Ice.Default.Router");
#endif

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

#ifdef ICEE_HAS_LOCATOR
    const_cast<string&>(defaultLocator) = properties->getProperty("Ice.Default.Locator");
#endif
}
