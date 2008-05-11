// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_SAFE_STDIO_H
#define ICEE_SAFE_STDIO_H

#include <IceE/Config.h>

namespace Ice
{

//
// This is for two reasons.
// 1. snprintf is _snprintf under windows.
// 2. This function ensures the buffer is null terminated.
//
ICE_API std::string printfToString(const char*, ...);

}

#endif
