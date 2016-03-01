// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

    const int slicing;
    const char* slicingCat;

    const int gc;
    const char* gcCat;

    const int threadPool;
    const char* threadPoolCat;
};

}

#endif
