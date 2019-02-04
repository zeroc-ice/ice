//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/InputUtil.h>
#include <stdlib.h>

using namespace std;
using namespace IceUtil;

namespace IceUtilInternal
{

Int64
strToInt64(const char* s, char** endptr, int base)
{
#if defined(_WIN32) && defined(_MSC_VER)
    return _strtoi64(s, endptr, base);
#elif defined(ICE_64) && !defined(_WIN32)
    return strtol(s, endptr, base);
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
