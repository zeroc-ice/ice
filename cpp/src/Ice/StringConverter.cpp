// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/StringConverter.h>
#include <IceUtil/Unicode.h>
#include <Ice/LocalException.h>

using namespace IceUtil;
using namespace std;

namespace Ice
{

Byte* 
UnicodeWstringConverter::toUTF8(const wchar_t* sourceStart, 
				const wchar_t* sourceEnd,
				UTF8Buffer& buffer) const
{
    //
    // The "chunk size" is the maximum of the number of characters in the
    // source and 6 (== max bytes necessary to encode one Unicode character).
    //
    size_t chunkSize = std::max<size_t>(static_cast<size_t>(sourceEnd - sourceStart), 6);

    Byte* targetStart = buffer.getMoreBytes(chunkSize, 0);
    Byte* targetEnd = targetStart + chunkSize;

    ConversionResult result;

    while((result =
	  convertUTFWstringToUTF8(sourceStart, sourceEnd, 
				  targetStart, targetEnd, lenientConversion))
	  == targetExhausted)
    {
	targetStart = buffer.getMoreBytes(chunkSize, targetStart);
	targetEnd = targetStart + chunkSize;
    }
	
    switch(result)
    {
	case conversionOK:
	    break;
	case sourceExhausted:
	    throw StringConversionException(__FILE__, __LINE__, "wide string source exhausted");
	case sourceIllegal:
	    throw StringConversionException(__FILE__, __LINE__, "wide string source illegal");
	default:
	{
	    assert(0);
	    throw StringConversionException(__FILE__, __LINE__);
	}
    }
    return targetStart;
}

void 
UnicodeWstringConverter::fromUTF8(const Byte* sourceStart, const Byte* sourceEnd,
				  const wchar_t*& targetStart, const wchar_t*& targetEnd) const
{
    size_t size = static_cast<size_t>(sourceEnd - sourceStart);
	
    wchar_t* outBuf = new wchar_t[size];
    wchar_t* tStart = outBuf;
    
    ConversionResult result = convertUTF8ToUTFWstring(
	sourceStart, sourceEnd, tStart, outBuf + size, 
	lenientConversion);

    switch(result)
    {
	case conversionOK:
	{
	    targetStart = outBuf;
	    targetEnd = tStart;
	    break;
	}
	case sourceExhausted:
	    throw StringConversionException(__FILE__, __LINE__, "UTF-8 string source exhausted");
	case sourceIllegal:
	    throw StringConversionException(__FILE__, __LINE__, "UTF-8 string source illegal");
	default:
	{
	    assert(0);
	    throw StringConversionException(__FILE__, __LINE__);
	}
    }
}

void 
UnicodeWstringConverter::freeTarget(const wchar_t* targetStart) const
{
#if defined(_MSC_VER) && _MSC_VER < 1300
    delete[] const_cast<wchar_t*>(targetStart);
#else
    delete[] targetStart;
#endif
}

void 
UnicodeWstringConverter::fromUTF8(const Byte* sourceStart, const Byte* sourceEnd,
				  wstring& target) const
{
    ConversionResult result = 
	convertUTF8ToUTFWstring(sourceStart, sourceEnd, target, lenientConversion);

    switch(result)
    {	 
	case conversionOK:
	    break;
	case sourceExhausted:
	    throw StringConversionException(__FILE__, __LINE__, "UTF-8 string source exhausted");
	case sourceIllegal:
	    throw StringConversionException(__FILE__, __LINE__, "UTF-8 string source illegal");
	default:
	{
	    assert(0);
	    throw StringConversionException(__FILE__, __LINE__);
	}
    }
}
}
