// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/DefaultsAndOverrides.h>
#include <IceE/Properties.h>
#include <IceE/Network.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(DefaultsAndOverrides* p) { p->__incRef(); }
void IceInternal::decRef(DefaultsAndOverrides* p) { p->__decRef(); }

IceInternal::DefaultsAndOverrides::DefaultsAndOverrides(const PropertiesPtr& properties) :
    overrideTimeout(false),
    overrideTimeoutValue(-1),
    overrideConnectTimeout(false),
    overrideConnectTimeoutValue(-1)
{
    const_cast<string&>(defaultHost) = properties->getProperty("IceE.Default.Host");
    if(defaultHost.empty())
    {
	const_cast<string&>(defaultHost) = getLocalHost(true);
    }

#ifndef ICEE_NO_ROUTER
    const_cast<string&>(defaultRouter) = properties->getProperty("IceE.Default.Router");
#endif

    string value;
    
    value = properties->getProperty("IceE.Override.Timeout");
    if(!value.empty())
    {
	const_cast<bool&>(overrideTimeout) = true;
	const_cast<Int&>(overrideTimeoutValue) = properties->getPropertyAsInt("IceE.Override.Timeout");
    }

    value = properties->getProperty("IceE.Override.ConnectTimeout");
    if(!value.empty())
    {
	const_cast<bool&>(overrideConnectTimeout) = true;
	const_cast<Int&>(overrideConnectTimeoutValue) = properties->getPropertyAsInt("IceE.Override.ConnectTimeout");
    }

#ifndef ICEE_NO_LOCATOR
    const_cast<string&>(defaultLocator) = properties->getProperty("IceE.Default.Locator");
#endif
}
