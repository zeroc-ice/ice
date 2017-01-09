// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_STRING_CONVERTER_H
#define ICE_UTIL_STRING_CONVERTER_H

#include <IceUtil/Config.h>
#include <IceUtil/Exception.h>
#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>

#include <string>

namespace IceUtil
{

//
// Provides bytes to toUTF8. Can raise std::bad_alloc or Ice::MemoryLimitException
// when too many bytes are requested.
//
class ICE_API UTF8Buffer
{
public:
    virtual Byte* getMoreBytes(size_t howMany, Byte* firstUnused) = 0;

    virtual ~UTF8Buffer();
};

//
// A StringConverter converts narrow or wide-strings to and from UTF-8 byte sequences.
// It's used by the communicator during marshaling (toUTF8) and unmarshaling (fromUTF8).
// It report errors by raising IllegalConversionException or an exception raised
// by UTF8Buffer
//

template<typename charT>
class BasicStringConverter
#ifndef ICE_CPP11_MAPPING
    : public IceUtil::Shared
#endif
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

    virtual ~BasicStringConverter()
    {
    }
};

typedef BasicStringConverter<char> StringConverter;
ICE_DEFINE_PTR(StringConverterPtr, StringConverter);

typedef BasicStringConverter<wchar_t> WstringConverter;
ICE_DEFINE_PTR(WstringConverterPtr, WstringConverter);

//
// Create a WstringConverter that converts to and from UTF-16 or UTF-32
// depending on sizeof(wchar_t).
//
//
ICE_API WstringConverterPtr createUnicodeWstringConverter();

//
// Retrieve the per process narrow string converter.
//
ICE_API StringConverterPtr getProcessStringConverter();

//
// Set the per process narrow string converter.
//
ICE_API void setProcessStringConverter(const StringConverterPtr&);

//
// Retrieve the per process wide string converter.
//
ICE_API WstringConverterPtr getProcessWstringConverter();

//
// Set the per process wide string converter.
//
ICE_API void setProcessWstringConverter(const WstringConverterPtr&);


//
// Converts the given wide string to a narrow string
//
// If the StringConverter parameter is null, the result's narrow
// string encoding is UTF-8.
// If the WstringConverter parameter is null, the input's wstring
// encoding is UTF-16 or UTF-32 depending on the size of wchar_t.
//
ICE_API std::string
wstringToString(const std::wstring&,
                const StringConverterPtr& = 0,
                const WstringConverterPtr& = 0);

//
// Converts the given narrow string to a wide string
//
// If the StringConverter parameter is null, the input's narrow string
// encoding is UTF-8.
// If the WstringConverter parameter is null, the result's wstring
// encoding is UTF-16 or UTF-32 depending on the size of wchar_t.
//
ICE_API std::wstring
stringToWstring(const std::string&,
                const StringConverterPtr& = 0,
                const WstringConverterPtr& = 0);


//
// Converts the given string from the native narrow string encoding to
// UTF-8 using the given converter. If the converter is null, returns
// the given string.
//
ICE_API std::string
nativeToUTF8(const std::string&, const StringConverterPtr&);

//
// Converts the given string from UTF-8 to the native narrow string
// encoding using the given converter. If the converter is null,
// returns the given string.
//
ICE_API std::string
UTF8ToNative(const std::string&, const StringConverterPtr&);

}

namespace IceUtilInternal
{
//
// Convert from UTF-8 to UTF-16/32
//
ICE_API std::vector<unsigned short> toUTF16(const std::vector<IceUtil::Byte>&);
ICE_API std::vector<unsigned int> toUTF32(const std::vector<IceUtil::Byte>&);

//
// Convert from UTF-32 to UTF-8
//
ICE_API std::vector<IceUtil::Byte> fromUTF32(const std::vector<unsigned int>&);

}

#ifdef _WIN32
namespace IceUtil
{
//
// Create a StringConverter that converts to and from narrow chars
// in the given code page, using MultiByteToWideChar and WideCharToMultiByte
//
ICE_API StringConverterPtr createWindowsStringConverter(unsigned int);

}
#endif

#endif
