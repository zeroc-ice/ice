// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceUtil/InputUtil.h>
#include <stdlib.h>

using namespace std;

namespace IceUtil
{

Int64
strToInt64(const char* s, char** endptr, int base)
{
#if defined(_WIN32)
    // TODO: WIN32 implementation is missing
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

    string nonWhite(i, j.base());	// nonWhite has leading and trailing whitespace stripped

    errno = 0;
    const char* startp = nonWhite.c_str();
    char* endp;
    result = strtoll(startp, &endp, 0);
    pos = *endp == '\0' ? string::npos : (i - stringToParse.begin()) + (endp - startp);
    return startp != endp && errno != ERANGE && errno != EINVAL;
}

}
