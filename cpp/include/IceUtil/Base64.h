// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_UTIL_BASE_64_H
#define ICE_UTIL_BASE_64_H

#include <IceUtil/Config.h>
#include <string>
#include <vector>

namespace IceUtil
{

using std::vector;
using std::string;

// Defined here to avoid using Ice::ByteSeq
typedef vector<char> ICE_UTIL_API ByteSeq;

class ICE_UTIL_API Base64
{
public:

    static string encode(const ByteSeq&);
    static ByteSeq decode(const string&);

private:

    static char encode(unsigned char);
    static unsigned char decode(char);
    static bool isBase64(char);
};

}

#endif
