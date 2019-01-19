//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_UNICODE_H
#define ICE_UTIL_UNICODE_H

#include <IceUtil/Config.h>
#include <vector>

namespace IceUtilInternal
{

//
// Convert UTF-8 byte-sequences to and from UTF-16 or UTF-32 (with native endianness)
//
// These are wrappers for Unicode's ConvertUTF.h/cpp.

//
// Convert wstring encoded with UTF-16 or UTF-32 to UTF-8.
// Returns false if needs for space and true upon success.
// Throws IllegalConversionException to report error
//
bool
convertUTFWstringToUTF8(const wchar_t*& sourceStart, const wchar_t* sourceEnd,
                        IceUtil::Byte*& targetStart, IceUtil::Byte* targetEnd);

void
convertUTF8ToUTFWstring(const IceUtil::Byte*& sourceStart, const IceUtil::Byte* sourceEnd,
                        std::wstring& target);

void
convertUTF8ToUTF16(const std::vector<unsigned char>&, std::vector<unsigned short>&);

void
convertUTF8ToUTF32(const std::vector<unsigned char>&, std::vector<unsigned int>&);

void
convertUTF32ToUTF8(const std::vector<unsigned int>&, std::vector<unsigned char>&);

}

#endif
