// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_UTIL_INPUT_UTIL_H
#define ICE_UTIL_INPUT_UTIL_H

#include <IceUtil/Config.h>
#include <string>
#include <limits.h>

namespace IceUtil
{

#if defined(_WIN32)
    typedef __int64 Int64;
    const Int64 INT64MIN = -9223372036854775808i64;
    const Int64 INT64MAX =  9223372036854775807i64;
#elif defined(__linux__) && defined(i386)
    typedef long long Int64;
    const Int64 INT64MIN = LONGLONG_MIN;
    const Int64 INT64MAX = LONGLONG_MAX;
#else
#   error "Unsupported operating system or platform!"
#endif

ICE_UTIL_API Int64 strToInt64(const char*, char**, int);
ICE_UTIL_API bool stringToInt64(const std::string&, Int64&, std::string::size_type&);

}

#endif
