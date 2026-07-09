// Copyright (c) ZeroC, Inc.

#ifndef ICE_IAP_UTIL_H
#define ICE_IAP_UTIL_H

#import <Foundation/NSString.h>

#include <string>

namespace IceObjC
{
    // Converts an NSString to a std::string, yielding "" when the string is nil or not representable as UTF-8.
    inline std::string nsToString(NSString* s)
    {
        const char* utf8 = [s UTF8String];
        return utf8 ? std::string{utf8} : std::string{};
    }
}

#endif
