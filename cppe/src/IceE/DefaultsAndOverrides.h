// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_DEFAULTS_AND_OVERRIDES_H
#define ICEE_DEFAULTS_AND_OVERRIDES_H

#include <IceE/DefaultsAndOverridesF.h>
#include <IceE/PropertiesF.h>
#include <IceE/Shared.h>

namespace IceInternal
{

class DefaultsAndOverrides : public ::IceUtil::Shared
{
public:

    DefaultsAndOverrides(const ::Ice::PropertiesPtr&);

    std::string defaultHost;
#ifdef ICEE_HAS_ROUTER
    std::string defaultRouter;
#endif
#ifdef ICEE_HAS_LOCATOR
    std::string defaultLocator;
#endif

    bool overrideTimeout;
    Ice::Int overrideTimeoutValue;
    bool overrideConnectTimeout;
    Ice::Int overrideConnectTimeoutValue;
};

}

#endif
