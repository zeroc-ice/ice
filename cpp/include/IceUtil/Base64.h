// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
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

class ICE_UTIL_API Base64
{
public:

    static std::string encode(const std::vector<char>&);
    static std::vector<char> decode(const std::string&);


private:

    static char encode(unsigned char);
    static unsigned char decode(char);
    static bool isBase64(char);
};

}

#endif
