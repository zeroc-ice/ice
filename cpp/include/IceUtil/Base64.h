// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

    static std::string encode(const std::vector<unsigned char>&);
    static std::vector<unsigned char> decode(const std::string&);
    static bool isBase64(char);

private:

    static char encode(unsigned char);
    static unsigned char decode(char);
};

}

#endif
