// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_UNICODE_H
#define ICE_UTIL_UNICODE_H

#include <IceUtil/Config.h>
#include <IceUtil/StringConverter.h>

namespace IceUtilInternal
{

//
// Converts UTF-8 byte-sequences to and from UTF-16 or UTF-32 (with native
// endianness) depending on sizeof(wchar_t).
//
// These are thin wrappers over the UTF8/16/32 converters provided by 
// unicode.org
//

enum ConversionResult
{
        conversionOK,           /* conversion successful */
        sourceExhausted,        /* partial character in source, but hit end */
        targetExhausted,        /* insuff. room in target for conversion */
        sourceIllegal           /* source sequence is illegal/malformed */
};

ConversionResult 
convertUTFWstringToUTF8(const wchar_t*& sourceStart, const wchar_t* sourceEnd, 
                        IceUtil::Byte*& targetStart, IceUtil::Byte* targetEnd, IceUtil::ConversionFlags flags);

ConversionResult
convertUTF8ToUTFWstring(const IceUtil::Byte*& sourceStart, const IceUtil::Byte* sourceEnd, 
                        wchar_t*& targetStart, wchar_t* targetEnd, IceUtil::ConversionFlags flags);

ConversionResult 
convertUTF8ToUTFWstring(const IceUtil::Byte*& sourceStart, const IceUtil::Byte* sourceEnd, 
                        std::wstring& target, IceUtil::ConversionFlags flags);


ICE_UTIL_API ConversionResult
convertUTF8ToUTF16(const std::vector<unsigned char>&, std::vector<unsigned short>&,
                   IceUtil::ConversionFlags);

ICE_UTIL_API ConversionResult
convertUTF8ToUTF32(const std::vector<unsigned char>&, std::vector<unsigned int>&,
                   IceUtil::ConversionFlags);

ICE_UTIL_API ConversionResult
convertUTF32ToUTF8(const std::vector<unsigned int>&, std::vector<unsigned char>&,
                   IceUtil::ConversionFlags);

}

#endif
