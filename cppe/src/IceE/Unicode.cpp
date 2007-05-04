// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Config.h>

#if defined(ICEE_HAS_WSTRING)

#include <IceE/Unicode.h>
#include <IceE/ConvertUTF.h>

using namespace std;
using namespace IceUtil;

namespace
{
//
// Helper class, base never defined
// Usage: WstringHelper<sizeof(wchar_t)>::toUTF8 and fromUTF8.
//
template<size_t wcharSize> 
struct WstringHelper
{
    static ConversionResult toUTF8(
	const wchar_t*& sourceStart, const wchar_t* sourceEnd, 
	Byte*& targetStart, Byte* targetEnd, ConversionFlags flags);

    static ConversionResult fromUTF8(
	const Byte*& sourceStart, const Byte* sourceEnd, 
	wchar_t*& targetStart, wchar_t* targetEnd, ConversionFlags flags);
};

template<>
struct WstringHelper<2>
{
    static ConversionResult toUTF8(
	const wchar_t*& sourceStart, const wchar_t* sourceEnd, 
	Byte*& targetStart, Byte* targetEnd, ConversionFlags flags)
    {
	return ConvertUTF16toUTF8(
	    reinterpret_cast<const UTF16**>(&sourceStart),
	    reinterpret_cast<const UTF16*>(sourceEnd),
	    &targetStart, targetEnd, flags);
    }
    
    static ConversionResult fromUTF8(
	const Byte*& sourceStart, const Byte* sourceEnd, 
	wchar_t*& targetStart, wchar_t* targetEnd, ConversionFlags flags)
    {
	return ConvertUTF8toUTF16(
	    &sourceStart, sourceEnd,
	    reinterpret_cast<UTF16**>(&targetStart),
	    reinterpret_cast<UTF16*>(targetEnd), flags);
    }
};

template<>
struct WstringHelper<4>
{
    static ConversionResult toUTF8(
	const wchar_t*& sourceStart, const wchar_t* sourceEnd, 
	Byte*& targetStart, Byte* targetEnd, ConversionFlags flags)
    {
	return ConvertUTF32toUTF8(
	    reinterpret_cast<const UTF32**>(&sourceStart),
	    reinterpret_cast<const UTF32*>(sourceEnd),
	    &targetStart, targetEnd, flags);
    }
    
    static ConversionResult fromUTF8(
	const Byte*& sourceStart, const Byte* sourceEnd, 
	wchar_t*& targetStart, wchar_t* targetEnd, ConversionFlags flags)
    {
	return ConvertUTF8toUTF32(
	    &sourceStart, sourceEnd,
	    reinterpret_cast<UTF32**>(&targetStart),
	    reinterpret_cast<UTF32*>(targetEnd), flags);
    }
};
}

//
// convertXXX functions
//

ConversionResult 
IceUtil::convertUTFWstringToUTF8(
    const wchar_t*& sourceStart, const wchar_t* sourceEnd, 
    Byte*& targetStart, Byte* targetEnd, ConversionFlags flags)
{
    return WstringHelper<sizeof(wchar_t)>::toUTF8(
	sourceStart, sourceEnd, targetStart, targetEnd, flags);
}

ConversionResult
IceUtil::convertUTF8ToUTFWstring(
    const Byte*& sourceStart, const Byte* sourceEnd, 
    wchar_t*& targetStart, wchar_t* targetEnd, ConversionFlags flags)
{
    return WstringHelper<sizeof(wchar_t)>::fromUTF8(
	sourceStart, sourceEnd, targetStart, targetEnd, flags);
}

ConversionResult 
IceUtil::convertUTF8ToUTFWstring(const Byte*& sourceStart, const Byte* sourceEnd, 
				 std::wstring& target, ConversionFlags flags)
{
    //
    // Could be reimplemented without this temporary wchar_t buffer
    //
    size_t size = static_cast<size_t>(sourceEnd - sourceStart);
    wchar_t* outBuf = new wchar_t[size];
    wchar_t* targetStart = outBuf; 
    wchar_t* targetEnd = targetStart + size;

    ConversionResult result =  
	convertUTF8ToUTFWstring(sourceStart, sourceEnd, targetStart,
				targetEnd, flags);

    if(result == conversionOK)
    {
	std::wstring s(outBuf, static_cast<size_t>(targetStart - outBuf));
	s.swap(target);
    }
    delete[] outBuf;
    return result;
}


//
// wstringToString and stringToWstring
//

const char* IceUtil::UTFConversionException::_name = "IceUtil::UTFConversionException";

IceUtil::UTFConversionException::UTFConversionException(const char* file, int line, 
							ConversionResult cr): 
    Exception(file, line),
    _conversionResult(cr)
{}

string
IceUtil::UTFConversionException::ice_name() const
{
    return _name;
}

string
IceUtil::UTFConversionException::toString() const
{
    string str = Exception::toString();
    switch(_conversionResult)
    {
	case sourceExhausted:
	    str += ": source exhausted";
	    break;
	case targetExhausted:
	    str += ": target exhausted";
	    break;
	case sourceIllegal:
	    str += ": illegal source";
	    break;
	default:
	    assert(0);
	    break;
    };
    return str;
}

IceUtil::Exception*
IceUtil::UTFConversionException::ice_clone() const
{
    return new UTFConversionException(*this);
}

void
IceUtil::UTFConversionException::ice_throw() const
{
    throw *this;
}

IceUtil::ConversionResult
IceUtil::UTFConversionException::conversionResult() const
{
    return _conversionResult;
}


string
IceUtil::wstringToString(const wstring& wstr)
{
    string target;
    
    size_t size = wstr.size() * 3 * (sizeof(wchar_t) / 2);

    Byte* outBuf = new Byte[size];
    Byte* targetStart = outBuf; 
    Byte* targetEnd = outBuf + size;

    const wchar_t* sourceStart = wstr.data();
  
    ConversionResult cr = 
	convertUTFWstringToUTF8(
	    sourceStart, sourceStart + wstr.size(), 
	    targetStart, targetEnd, lenientConversion);
	
    if(cr != conversionOK)
    {
	delete[] outBuf;
	throw UTFConversionException(__FILE__, __LINE__, cr);
    }
    
    string s(reinterpret_cast<char*>(outBuf),
	     static_cast<size_t>(targetStart - outBuf));
    s.swap(target);
    delete[] outBuf;
    return target;
}

wstring
IceUtil::stringToWstring(const string& str)
{
    wstring result;
    const Byte* sourceStart = reinterpret_cast<const Byte*>(str.data());
    
    ConversionResult cr 
	= convertUTF8ToUTFWstring(sourceStart, sourceStart + str.size(),
				  result, lenientConversion);

    if(cr != conversionOK)
    {
	throw UTFConversionException(__FILE__, __LINE__, cr);
    }
    return result;
}


#if defined(_MSC_VER) && (_MSC_VER >= 1300)
//
// See comments in IceUtil/Unicode.h
//

#   if _MSC_VER < 1400
string
IceUtil::wstringToString(const basic_string<__wchar_t>& str)
{
    return wstringToString(*reinterpret_cast<const wstring*>(&str));
}

basic_string<__wchar_t>
IceUtil::stringToNativeWstring(const string& str)
{
    return reinterpret_cast<basic_string<__wchar_t>& >(stringToWstring(str));
}
#   else
string
IceUtil::wstringToString(const basic_string<unsigned short>& str)
{
    return wstringToString(*reinterpret_cast<const wstring*>(&str));
}

basic_string<unsigned short>
IceUtil::stringToTypedefWstring(const string& str)
{
    return reinterpret_cast<basic_string<unsigned short>& >(stringToWstring(str));
}

#   endif
#endif

#endif
