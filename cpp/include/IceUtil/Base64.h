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

// Defined here to avoid using Ice::ByteSeq
typedef std::vector<char> ICE_UTIL_API ByteSeq;

class ICE_UTIL_API Base64
{
public:

    static std::string encode(const ByteSeq&);
    static ByteSeq decode(const std::string&);


private:

    static char encode(unsigned char);
    static unsigned char decode(char);
    static bool isBase64(char);
};

}

#endif
