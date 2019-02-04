//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/Config.h>

#ifndef ICE_HAS_CODECVT_UTF8
//
// It's better to exclude the file from the build, but it's not always
// easy to do.
//

#include <IceUtil/Unicode.h>
#include <IceUtil/Exception.h>

#include <IceUtil/ConvertUTF.h>

using namespace std;
using namespace IceUtil;
using namespace IceUtilInternal;

namespace
{
//
// Helper class, base never defined
// Usage: WstringHelper<sizeof(wchar_t)>::toUTF8 and fromUTF8.
//
template<size_t wcharSize> struct WstringHelper;

template<>
struct WstringHelper<2>
{
    static ConversionResult toUTF8(
        const wchar_t*& sourceStart, const wchar_t* sourceEnd,
        Byte*& targetStart, Byte* targetEnd)
    {
        return ConvertUTF16toUTF8(
            reinterpret_cast<const UTF16**>(&sourceStart),
            reinterpret_cast<const UTF16*>(sourceEnd),
            &targetStart, targetEnd, lenientConversion);
    }

    static ConversionResult fromUTF8(
        const Byte*& sourceStart, const Byte* sourceEnd,
        wchar_t*& targetStart, wchar_t* targetEnd)
    {
        return ConvertUTF8toUTF16(
            &sourceStart, sourceEnd,
            reinterpret_cast<UTF16**>(&targetStart),
            reinterpret_cast<UTF16*>(targetEnd), lenientConversion);
    }
};

template<>
struct WstringHelper<4>
{
    static ConversionResult toUTF8(
        const wchar_t*& sourceStart, const wchar_t* sourceEnd,
        Byte*& targetStart, Byte* targetEnd)
    {
        return ConvertUTF32toUTF8(
            reinterpret_cast<const UTF32**>(&sourceStart),
            reinterpret_cast<const UTF32*>(sourceEnd),
            &targetStart, targetEnd, lenientConversion);
    }

    static ConversionResult fromUTF8(
        const Byte*& sourceStart, const Byte* sourceEnd,
        wchar_t*& targetStart, wchar_t* targetEnd)
    {
        return ConvertUTF8toUTF32(
            &sourceStart, sourceEnd,
            reinterpret_cast<UTF32**>(&targetStart),
            reinterpret_cast<UTF32*>(targetEnd), lenientConversion);
    }
};

void checkResult(ConversionResult result)
{
    switch (result)
    {
        case conversionOK:
            break;
        case sourceExhausted:
            throw IceUtil::IllegalConversionException(__FILE__, __LINE__, "source exhausted");
        case sourceIllegal:
            throw IceUtil::IllegalConversionException(__FILE__, __LINE__, "source illegal");
        case targetExhausted:
            throw IceUtil::IllegalConversionException(__FILE__, __LINE__, "source illegal");
        default:
        {
            assert(0);
            throw IceUtil::IllegalConversionException(__FILE__, __LINE__);
        }
    }
}
}

//
// convertXXX functions
//

bool
IceUtilInternal::convertUTFWstringToUTF8(const wchar_t*& sourceStart, const wchar_t* sourceEnd,
                                         Byte*& targetStart, Byte* targetEnd)
{
    ConversionResult result = WstringHelper<sizeof(wchar_t)>::toUTF8(
        sourceStart, sourceEnd, targetStart, targetEnd);

    if(result == targetExhausted)
    {
        return false;
    }
    else
    {
        checkResult(result);
        return true;
    }
}

void
IceUtilInternal::convertUTF8ToUTFWstring(const Byte*& sourceStart, const Byte* sourceEnd, std::wstring& target)
{
    size_t sourceSize = static_cast<size_t>(sourceEnd - sourceStart);

    target.resize(sourceSize);
    wchar_t* targetStart = const_cast<wchar_t*>(target.data());
    wchar_t* targetEnd = targetStart + sourceSize;

    ConversionResult result = WstringHelper<sizeof(wchar_t)>::fromUTF8(sourceStart, sourceEnd,
                                                                       targetStart, targetEnd);

    checkResult(result);
    target.resize(targetStart - target.data());
}

void
IceUtilInternal::convertUTF8ToUTF16(const vector<unsigned char>& source, vector<unsigned short>& target)
{
    target.resize(source.size());
    const unsigned char* sourceStart = &source[0];
    const unsigned char* sourceEnd = &source[0] + source.size();

    unsigned short* targetStart = &target[0];
    unsigned short* targetEnd = &target[0] + target.size();
    ConversionResult result = ConvertUTF8toUTF16(&sourceStart, sourceEnd, &targetStart, targetEnd, lenientConversion);

    checkResult(result);
    target.resize(targetStart - &target[0]);
}

void
IceUtilInternal::convertUTF8ToUTF32(const vector<unsigned char>& source, vector<unsigned int>& target)
{
    target.resize(source.size());
    const unsigned char* sourceStart = &source[0];
    const unsigned char* sourceEnd = &source[0] + source.size();

    unsigned int* targetStart = &target[0];
    unsigned int* targetEnd = &target[0] + target.size();
    ConversionResult result = ConvertUTF8toUTF32(&sourceStart, sourceEnd, &targetStart, targetEnd, lenientConversion);

    checkResult(result);
    target.resize(targetStart - &target[0]);
}

void
IceUtilInternal::convertUTF32ToUTF8(const vector<unsigned int>& source, vector<unsigned char>& target)
{
    target.resize(source.size() * 4);

    const unsigned int* sourceStart = &source[0];
    const unsigned int* sourceEnd = &source[0] + source.size();

    unsigned char* targetStart = &target[0];
    unsigned char* targetEnd = &target[0] + target.size();
    ConversionResult result = ConvertUTF32toUTF8(&sourceStart, sourceEnd, &targetStart, targetEnd, lenientConversion);

    checkResult(result);
    target.resize(targetStart - &target[0]);
}

#endif
