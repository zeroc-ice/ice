// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_ICE_DEFAULTS_AND_OVERRIDES_F_H
#define ICE_ICE_DEFAULTS_AND_OVERRIDES_F_H

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

    bool overrideTimeout;
    Ice::Int overrideTimeoutValue;
    bool overrideComppress;
    bool overrideComppressValue;
};

}

#endif
