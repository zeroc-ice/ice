// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/InputUtil.h>
#include <stdlib.h>
#include <errno.h>

#if defined(_MSC_VER) && (_MSC_VER < 1300) 
#include <limits.h>
#endif

#if defined(__hpux)
#include <inttypes.h>
#endif

using namespace std;

namespace IceUtil
{

#if defined(__BCPLUSPLUS__) || (defined(_MSC_VER) && (_MSC_VER < 1300))
//
// The VC60 runtime does not include _strtoi64, so we provide our own implementation
//

static const string allDigits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

//
// Table to convert ASCII digits/letters into their value (100 for unused slots)
//
static const char digitVal[] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,               // '0' - '9'
    100, 100, 100, 100, 100, 100, 100,          // punctuation
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,     // 'A' - 'J'
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,     // 'K' - 'T'
    30, 31, 32, 33, 34, 35                      // 'U' - 'Z'
};

static IceUtil::Int64
strToInt64Impl(const char* s, char** endptr, int base)
{
    //
    // Assume nothing will be there to convert for now
    //
    if(endptr)
    {
        *endptr = const_cast<char *>(s);
    }

    //
    // Skip leading whitespace
    //
    while(*s && isspace(*s))
    {
        ++s;
    }

    //
    // Check for sign
    //
    int sign = 1;
    if(*s == '+')
    {
        ++s;
    }
    else if(*s == '-')
    {
        sign = -1;
        ++s;
    }

    //
    // Check that base is valid
    //
    if(base == 0)
    {
        if(*s == '0')
        {
            base = 8;
            ++s;
            
            //
            // We have at least this zero
            //
            if(endptr)
            {
                *endptr = const_cast<char *>(s);
            }
            
            if(*s == 'x' || *s == 'X')
            {
                base = 16;
                ++s;
            }
        }
        else
        {
            base = 10;
        }
    }
    else if(base < 2 || base > 36)
    {
        errno = EINVAL;
        return 0;
    }

    //
    // Check that we have something left to parse
    //
    if(*s == '\0')
    {
        //
        // We did not read any new digit so we don't update endptr
        //
        return 0;
    }

    Int64 result = 0;
    bool overflow = false;
    bool digitFound = false;
    const string validDigits(allDigits.begin(), allDigits.begin() + base);
    while(*s && validDigits.find_first_of(toupper(*s)) != validDigits.npos)
    {   
        digitFound = true;
        if(!overflow)
        {
            int digit = digitVal[toupper(*s) - '0'];
            assert(digit != 100);
            if(result < _I64_MAX / base)
            {
                result *= base;
                result += digit;
            }
            else if((digit <= _I64_MAX % base) || (sign == -1 && digit == _I64_MAX % base + 1))
            {
                result *= base;
                result += digit;
            }
            else
            {
                overflow = true;
                result = sign == -1 ? _I64_MIN : _I64_MAX;
            }
        }
        ++s;
    }

    if(overflow)
    {
        errno = ERANGE;
    }
    else
    {
        result *= sign;
    }

    if(digitFound && endptr != 0)
    {
        *endptr = const_cast<char *>(s);
    }
 
    return result;
}

#endif


Int64
strToInt64(const char* s, char** endptr, int base)
{
#if defined(_WIN32)
#   if defined(__BCPLUSPLUS__) || (defined(_MSC_VER) && (_MSC_VER < 1300))
    return strToInt64Impl(s, endptr, base);
#   else
    return _strtoi64(s, endptr, base);
#   endif
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
