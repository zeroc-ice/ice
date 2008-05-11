// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/DisableWarnings.h>
#include <IceE/SafeStdio.h>
#include <stdarg.h>
#include <stdio.h>

using namespace std;

string
Ice::printfToString(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char buf[1024];
#if defined(_WIN32)
    _vsnprintf(buf, sizeof(buf)-1, fmt, ap);
#else
    vsnprintf(buf, sizeof(buf)-1, fmt, ap);
#endif
    buf[sizeof(buf)-1] = '\0';
    va_end(ap);

    return buf;
}

