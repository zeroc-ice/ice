// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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

enum ConversionFlags
{
    strictConversion = 0,
    lenientConversion
};

ICE_UTIL_API std::string wstringToString(const std::wstring&, ConversionFlags = lenientConversion);
ICE_UTIL_API std::wstring stringToWstring(const std::string&, ConversionFlags = lenientConversion);

typedef unsigned char Byte;

ICE_UTIL_API bool
isLegalUTF8Sequence(const Byte* source, const Byte* end);

enum ConversionError
{
    partialCharacter,
    badEncoding
};

//
// UTFConversionException is raised by wstringToString() or stringToWstring()
// to report a conversion error 
//
class ICE_UTIL_API UTFConversionException : public Exception
{
public:
    
    UTFConversionException(const char*, int, ConversionError);
    virtual std::string ice_name() const;
    virtual void ice_print(std::ostream&) const;
    virtual UTFConversionException* ice_clone() const;
    virtual void ice_throw() const;

    ConversionError conversionError() const;
private:

    const ConversionError _conversionError;
    static const char* _name;    
};

}

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

ICE_UTIL_API ConversionResult 
convertUTFWstringToUTF8(const wchar_t*& sourceStart, const wchar_t* sourceEnd, 
                        IceUtil::Byte*& targetStart, IceUtil::Byte* targetEnd, IceUtil::ConversionFlags flags);

ICE_UTIL_API ConversionResult
convertUTF8ToUTFWstring(const IceUtil::Byte*& sourceStart, const IceUtil::Byte* sourceEnd, 
                        wchar_t*& targetStart, wchar_t* targetEnd, IceUtil::ConversionFlags flags);

ICE_UTIL_API ConversionResult 
convertUTF8ToUTFWstring(const IceUtil::Byte*& sourceStart, const IceUtil::Byte* sourceEnd, 
                        std::wstring& target, IceUtil::ConversionFlags flags);

}

#endif
