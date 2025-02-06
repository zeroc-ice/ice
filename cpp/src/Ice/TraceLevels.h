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

        const int dispatch{0};
        const char* dispatchCat{"Dispatch"};

        const int network{0};
        const char* networkCat{"Network"};

        const int protocol{0};
        const char* protocolCat{"Protocol"};

        const int retry{0};
        const char* retryCat{"Retry"};

        const int location{0};
        const char* locationCat{"Locator"};

        const int slicing{0};
        const char* slicingCat{"Slicing"};

        const int threadPool{0};
        const char* threadPoolCat{"ThreadPool"};
    };
}

#endif
