// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_ICE_DEFAULTS_AND_OVERWRITES_F_H
#define ICE_ICE_DEFAULTS_AND_OVERWRITES_F_H

#include <IceUtil/Shared.h>
#include <Ice/DefaultsAndOverwritesF.h>
#include <Ice/PropertiesF.h>

namespace IceInternal
{

class DefaultsAndOverwrites : public ::IceUtil::Shared
{
public:

    DefaultsAndOverwrites(const ::Ice::PropertiesPtr&);

    std::string defaultHost;
    std::string defaultProtocol;
    std::string defaultRouter;

    bool overwriteTimeout;
    Ice::Int overwriteTimeoutValue;
    bool overwriteCompress;
    bool overwriteCompressValue;
};

}

#endif
