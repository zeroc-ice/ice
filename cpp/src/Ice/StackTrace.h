//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_STACK_TRACE_H
#define ICE_STACK_TRACE_H

#include "Ice/Config.h"

namespace IceInternal
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
