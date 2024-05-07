//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SSL_UTIL_H
#define ICE_SSL_UTIL_H

#include <functional>
#include <string>
#include <vector>

#if defined(__APPLE__)
#    include <CoreFoundation/CoreFoundation.h>
#    if TARGET_OS_IPHONE != 0
#        define ICE_USE_SECURE_TRANSPORT_IOS 1
#    else
#        define ICE_USE_SECURE_TRANSPORT_MACOS 1
#    endif
#endif

namespace Ice::SSL
{
#if defined(__APPLE__)
    //
    // Helper functions to use by Secure Transport.
    //
    std::string fromCFString(CFStringRef);

    inline CFStringRef toCFString(const std::string& s)
    {
        return CFStringCreateWithCString(nullptr, s.c_str(), kCFStringEncodingUTF8);
    }
#endif

    // Constants for X509 certificate alt names (AltNameOther, AltNameORAddress, AltNameEDIPartyName and
    // AltNameObjectIdentifier) are not supported.

    // const int AltNameOther = 0;
    const int AltNameEmail = 1;
    const int AltNameDNS = 2;
    // const int AltNameORAddress = 3;
    const int AltNameDirectory = 4;
    // const int AltNameEDIPartyName = 5;
    const int AltNameURL = 6;
    const int AltNAmeIP = 7;
    // const AltNameObjectIdentifier = 8;

    // Read a file into memory buffer.
    ICE_API void readFile(const std::string&, std::vector<char>&);

    // Determine if a file or directory exists, with an optional default directory.
    ICE_API bool checkPath(const std::string&, const std::string&, bool, std::string&);

    ICE_API bool parseBytes(const std::string&, std::vector<unsigned char>&);
}

#endif
