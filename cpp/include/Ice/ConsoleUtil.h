// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_CONSOLE_UTIL_H
#define ICE_CONSOLE_UTIL_H

#include <IceUtil/ConsoleUtil.h>

namespace IceInternal
{

using IceUtilInternal::consoleOut;
using IceUtilInternal::consoleErr;

#if !defined(__MINGW32__) && defined(_WIN32) && !defined(ICE_OS_UWP)
using IceUtilInternal::endl;
using IceUtilInternal::flush;
#endif

}

#endif
