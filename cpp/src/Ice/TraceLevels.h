// **********************************************************************
//
// Copyright (c) 2003
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

#ifndef ICE_TRACE_LEVELS_H
#define ICE_TRACE_LEVELS_H

#include <IceUtil/Shared.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/PropertiesF.h>

namespace IceInternal
{

class TraceLevels : public ::IceUtil::Shared
{
public:

    TraceLevels(const ::Ice::PropertiesPtr&);

    const int network;
    const char* networkCat;

    const int protocol;
    const char* protocolCat;

    const int retry;
    const char* retryCat;

    const int location;
    const char* locationCat;
};

}

#endif
