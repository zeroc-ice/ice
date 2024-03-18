//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_CONSOLE_UTIL_H
#define ICE_CONSOLE_UTIL_H

#include "IceUtil/ConsoleUtil.h"

namespace IceInternal
{
    using IceUtilInternal::consoleErr;
    using IceUtilInternal::consoleOut;

#if defined(_WIN32)
    using IceUtilInternal::endl;
    using IceUtilInternal::flush;
#endif
}

#endif
