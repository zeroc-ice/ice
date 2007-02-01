// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_UNICODE_H
#define ICE_UTIL_UNICODE_H

#include <IceUtil/Config.h>
#include <IceUtil/Exception.h>

namespace IceUtil
{

#if defined(_MSC_VER) && (_MSC_VER >= 1300)

//
// With Visual C++ 7.x, wchar_t is either a typedef unsigned short or a 
// native type (when /Zc:wchar_t is used).
// Since wstring is a typedef basic_string<wchar_t>, its type is also 
// different depending on whether /Zc:wchar_t is used or not.
//
// With Visual C++ 7.x, the default is typedef; with Visual C++ 8.0,
// the default is native type.
//
// Ice is always built with the default, but provides wstringToString() 
// and stringToWstring() implementations for both flavors of wstring.
//

#   if defined(_NATIVE_WCHAR_T_DEFINED)
ICE_UTIL_API std::string wstringToString(const std::wstring&);

#      if _MSC_VER >= 1400
//
// Building or using with VC8
//
ICE_UTIL_API std::wstring stringToWstring(const std::string&);
ICE_UTIL_API std::string wstringToString(const std::basic_string<unsigned short>&);
ICE_UTIL_API std::basic_string<unsigned short> stringToTypedefWstring(const std::string&);
#     else
//
// Using a VC7.x build with the non-default /Zc
//
ICE_UTIL_API std::wstring stringToNativeWstring(const std::string&);
inline std::wstring 
stringToWstring(const std::string& str)
{
    return stringToNativeWstring(str);
}
#     endif

#   else
ICE_UTIL_API std::string wstringToString(const std::wstring&);

#      if _MSC_VER < 1400
//
// Building or using with VC7.x
//
ICE_UTIL_API std::wstring stringToWstring(const std::string&);
ICE_UTIL_API std::string wstringToString(const std::basic_string<__wchar_t>&);
ICE_UTIL_API std::basic_string<__wchar_t> stringToNativeWstring(const std::string&);
#      else
//
// Using a VC8.x build the non-default /Zc
//
ICE_UTIL_API std::wstring stringToTypedefWstring(const std::string&);
inline std::wstring 
stringToWstring(const std::string& str)
{
    return stringToTypedefWstring(str);
}
#      endif
#   endif

#else

ICE_UTIL_API std::string wstringToString(const std::wstring&);
ICE_UTIL_API std::wstring stringToWstring(const std::string&);

#endif


//
// Converts UTF-8 byte-sequences to and from UTF-16 or UTF-32 (with native
// endianness) depending on sizeof(wchar_t).
//
// These are thin wrappers over the UTF8/16/32 converters provided by 
// unicode.org
//
//
// TODO: provide the same support for /Zc:wchar_t as the functions above
//

enum ConversionResult
{
        conversionOK,           /* conversion successful */
        sourceExhausted,        /* partial character in source, but hit end */
        targetExhausted,        /* insuff. room in target for conversion */
        sourceIllegal           /* source sequence is illegal/malformed */
};


enum ConversionFlags 
{
    strictConversion = 0,
    lenientConversion
};

typedef unsigned char Byte;

ICE_UTIL_API bool
isLegalUTF8Sequence(const Byte* source, const Byte* end);

ICE_UTIL_API ConversionResult 
convertUTFWstringToUTF8(const wchar_t*& sourceStart, const wchar_t* sourceEnd, 
                        Byte*& targetStart, Byte* targetEnd, ConversionFlags flags);

ICE_UTIL_API ConversionResult
convertUTF8ToUTFWstring(const Byte*& sourceStart, const Byte* sourceEnd, 
                        wchar_t*& targetStart, wchar_t* targetEnd, ConversionFlags flags);

ICE_UTIL_API ConversionResult 
convertUTF8ToUTFWstring(const Byte*& sourceStart, const Byte* sourceEnd, 
                        std::wstring& target, ConversionFlags flags);




//
// UTFConversionException is raised by wstringToString() or stringToWstring()
// to report a conversion error 
//
class ICE_UTIL_API UTFConversionException : public Exception
{
public:
    
    UTFConversionException(const char*, int, ConversionResult);
    virtual std::string ice_name() const;
    virtual void ice_print(std::ostream&) const;
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;

    ConversionResult conversionResult() const;
private:

    const ConversionResult _conversionResult;
    static const char* _name;    
};

}

#endif
