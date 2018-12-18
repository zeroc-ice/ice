// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_CONSOLE_UTIL_H
#define ICE_CONSOLE_UTIL_H

#include <IceUtil/ConsoleUtil.h>

namespace IceInternal
{

using IceUtilInternal::consoleOut;
using IceUtilInternal::consoleErr;

#if defined(_WIN32) && !defined(ICE_OS_UWP)
using IceUtilInternal::endl;
using IceUtilInternal::flush;
#endif

}

#endif
