// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_BASE_64_H
#define ICEE_BASE_64_H

#include <IceE/Config.h>
#include <string>
#include <vector>

namespace IceUtil
{

class Base64
{
public:

    ICE_API static std::string encode(const std::vector<unsigned char>&);
    ICE_API static std::vector<unsigned char> decode(const std::string&);
    ICE_API static bool isBase64(char);

private:

    static char encode(unsigned char);
    static unsigned char decode(char);
};

}

#endif
