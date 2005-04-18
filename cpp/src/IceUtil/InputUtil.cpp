// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/InputUtil.h>
#include <stdlib.h>
#include <errno.h>

using namespace std;


namespace IceUtil
{

Int64
strToInt64(const char* s, char** endptr, int base)
{
#if defined(_WIN32)
    return _strtoi64(s, endptr, base);
#elif defined(ICE_64)
    return strtol(s, endptr, base);
#elif defined(__hpux)
    return __strtoll(s, endptr, base);
#else
    return strtoll(s, endptr, base);
#endif
}

bool
stringToInt64(const string& s, Int64& result)
{
    const char* start = s.c_str();
    char* end = 0;
    errno = 0;
    result = strToInt64(start, &end, 0);
    return (errno == 0 && start != end);
}

}
