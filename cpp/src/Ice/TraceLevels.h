// Copyright (c) ZeroC, Inc.

#ifndef ICE_TRACE_LEVELS_H
#define ICE_TRACE_LEVELS_H

#include "Ice/PropertiesF.h"
#include "TraceLevelsF.h"

namespace IceInternal
{
    class TraceLevels
    {
    public:
        TraceLevels(const Ice::PropertiesPtr&);

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

        const int threadPool;
        const char* threadPoolCat;
    };
}

#endif
