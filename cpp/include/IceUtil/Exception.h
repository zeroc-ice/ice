//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_EXCEPTION_H
#define ICE_UTIL_EXCEPTION_H

#include "Config.h"

namespace IceUtilInternal
{
    enum StackTraceImpl
    {
        STNone,
        STDbghelp,
        STLibbacktrace,
        STLibbacktracePlus,
        STBacktrace
    };

    ICE_API StackTraceImpl stackTraceImpl();
}

#endif
