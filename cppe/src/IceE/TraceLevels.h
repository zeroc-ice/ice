// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_TRACE_LEVELS_H
#define ICEE_TRACE_LEVELS_H

#include <IceE/Shared.h>
#include <IceE/TraceLevelsF.h>
#include <IceE/PropertiesF.h>

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
