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

static const string allDigits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

//
// Table to convert ASCII digits/letters into their value (100 for unused slots)
//
static const char digitVal[] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,		// '0' - '9'
    100, 100, 100, 100, 100, 100, 100,		// punctuation
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,	// 'A' - 'J'
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,	// 'K' - 'T'
    30, 31, 32, 33, 34, 35			// 'U' - 'Z'
};

namespace IceUtil
{

//
// strToInt64 emulates strtoll() for Windows
//

Int64
strToInt64(const char* s, char** endptr, int base)
{
#if defined(_WIN32)
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
	    if(*++s == 'x' || *s == 'X')
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
    if(*s == '/0')
    {
	return 0;
    }

    Int64 result = 0;
    bool overflow = false;
    const string validDigits(allDigits.begin(), allDigits.begin() + base);
    while(*s && validDigits.find_first_of(toupper(*s)) != validDigits.npos)
    {	
	if(!overflow)
	{
	    int digit = digitVal[toupper(*s) - '0'];
	    assert(digit != 100);
	    if(result < Int64Max / base)
	    {
		result *= base;
		result += digit;
	    }
	    else if((digit <= Int64Max % base) || (sign == -1 && digit == Int64Max % base + 1))
	    {
		result *= base;
		result += digit;
	    }
	    else
	    {
		overflow = true;
		result = sign == -1 ? Int64Min : Int64Max;
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

    if(endptr)
    {
	*endptr = const_cast<char *>(s);
    }

    return result;

#elif defined(ICE_64)
    return strtol(s, endptr, base);
#elif defined(__hpux)
    return __strtoll(s, endptr, base);
#else
    return strtoll(s, endptr, base);
#endif
}

bool
stringToInt64(const string& stringToParse, Int64& result, string::size_type& pos)
{
    string::const_iterator i = stringToParse.begin();
    while(i != stringToParse.end() && isspace(*i))
    {
	++i;
    }
    if(i == stringToParse.end())	// String empty or nothing but whitespace
    {
	result = 0;
	pos = string::npos;
	return false;
    }
    string::const_reverse_iterator j = stringToParse.rbegin();
    while(isspace(*j))
    {
	++j;
    }					// j now points at last non-whitespace char

    string nonWhite(i, j.base());	// nonWhite has trailing whitespace stripped

    errno = 0;
    const char* startp = nonWhite.c_str();
    char* endp;
    result = strToInt64(startp, &endp, 0);
    pos = *endp == '\0' ? string::npos : (i - stringToParse.begin()) + (endp - startp);
    return startp != endp && errno != ERANGE && errno != EINVAL;
}

}
