// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_DEFAULTS_AND_OVERRIDES_H
#define ICE_DEFAULTS_AND_OVERRIDES_H

#include <IceUtil/Shared.h>
#include <Ice/DefaultsAndOverridesF.h>
#include <Ice/PropertiesF.h>

namespace IceInternal
{

class DefaultsAndOverrides : public ::IceUtil::Shared
{
public:

    DefaultsAndOverrides(const ::Ice::PropertiesPtr&);

    std::string defaultHost;
    std::string defaultProtocol;
    std::string defaultRouter;
    std::string defaultLocator;

    bool overrideTimeout;
    Ice::Int overrideTimeoutValue;
    bool overrideComppress;
    bool overrideComppressValue;
};

}

#endif
