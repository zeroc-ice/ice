// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STRING_CONVERTER_H
#define ICE_STRING_CONVERTER_H

#include <IceE/Config.h>

#ifdef ICEE_HAS_WSTRING

#include <IceE/Exception.h>
#include <IceE/Shared.h>
#include <IceE/Handle.h>

#include <string>

namespace Ice
{

//
// Provides bytes to toUTF8. Raises MemoryLimitException when too many
// bytes are requested.
//
class ICE_API UTF8Buffer
{
public:
    virtual Byte* getMoreBytes(size_t howMany, Byte* firstUnused) = 0;
    
    virtual ~UTF8Buffer() {}
};

//
// A StringConverter converts narrow or wide-strings to and from UTF-8 byte sequences.
// It's used by the communicator during marshaling (toUTF8) and unmarshaling (fromUTF8).
// It report errors by raising StringConversionFailed or MemoryLimitException.
//
template<typename charT>
class BasicStringConverter : public IceUtil::Shared
{
public:
    
    //
    // Returns a pointer to byte after the last written byte (which may be
    // past the last byte returned by getMoreBytes).
    //
    virtual Byte* toUTF8(const charT* sourceStart, const charT* sourceEnd,
			 UTF8Buffer&) const = 0;

    //
    // Unmarshals a UTF-8 sequence into a basic_string
    //
    virtual void fromUTF8(const Byte* sourceStart, const Byte* sourceEnd,
			  std::basic_string<charT>& target) const = 0;
};

typedef BasicStringConverter<char> StringConverter;
typedef IceUtil::Handle<StringConverter> StringConverterPtr;

typedef BasicStringConverter<wchar_t> WstringConverter;
typedef IceUtil::Handle<WstringConverter> WstringConverterPtr;

//
// Converts to and from UTF-16 or UTF-32 depending on sizeof(wchar_t)
//
class ICE_API UnicodeWstringConverter : public WstringConverter
{
public:

    virtual Byte* toUTF8(const wchar_t* sourceStart, const wchar_t* sourceEnd,
                         UTF8Buffer&) const;

    virtual void fromUTF8(const Byte* sourceStart, const Byte* sourceEnd,
                          std::wstring& target) const;
};

}
#endif

#endif
