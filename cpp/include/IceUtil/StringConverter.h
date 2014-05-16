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

enum ConversionFlags
{
    strictConversion = 0,
    lenientConversion
};

typedef unsigned char Byte;

ICE_UTIL_API bool
isLegalUTF8Sequence(const Byte* source, const Byte* end);

//
// Provides bytes to toUTF8. Can raise std::bad_alloc or Ice::MemoryLimitException 
// when too many bytes are requested.
//
class ICE_UTIL_API UTF8Buffer
{
public:
    virtual Byte* getMoreBytes(size_t howMany, Byte* firstUnused) = 0;
    
    virtual ~UTF8Buffer() {}
};

//
// A StringConverter converts narrow or wide-strings to and from UTF-8 byte sequences.
// It's used by the communicator during marshaling (toUTF8) and unmarshaling (fromUTF8).
// It report errors by raising IllegalConversionException or an exception raised
// by UTF8Buffer
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
class ICE_UTIL_API UnicodeWstringConverter : public WstringConverter
{
public:

    UnicodeWstringConverter(ConversionFlags = lenientConversion);

    virtual Byte* toUTF8(const wchar_t*, const wchar_t*, UTF8Buffer&) const;

    virtual void fromUTF8(const Byte*, const Byte*, std::wstring&) const;

private:
    const ConversionFlags _conversionFlags;
};

#ifdef _WIN32

//
// Converts to/from UTF-8 using MultiByteToWideChar and WideCharToMultiByte
//
class ICE_UTIL_API WindowsStringConverter : public StringConverter
{
public:

    explicit WindowsStringConverter(unsigned int);

    virtual Byte* toUTF8(const char*, const char*, UTF8Buffer&) const;

    virtual void fromUTF8(const Byte*, const Byte*, std::string& target) const;

private:
    unsigned int _cp;
    UnicodeWstringConverter _unicodeWstringConverter;
};
#endif

//
// Retrieve the per process narrow string converter.
//
ICE_UTIL_API StringConverterPtr getProcessStringConverter();

//
// Set the per process narrow string converter.
//
ICE_UTIL_API void setProcessStringConverter(const StringConverterPtr&);

//
// Retrieve the per process wide string converter.
//
ICE_UTIL_API WstringConverterPtr getProcessWstringConverter();

//
// Set the per process wide string converter.
//
ICE_UTIL_API void setProcessWstringConverter(const WstringConverterPtr&);


//
// Converts the given wide string to a narrow string
//
// If the StringConverter parameter is null, the result's narrow 
// string encoding is UTF-8.  
// If the WstringConverter parameter is null, the input's wstring 
// encoding is UTF-16 or UTF-32 depending on the size of wchar_t.
//
ICE_UTIL_API std::string
wstringToString(const std::wstring&, const StringConverterPtr& = 0, 
                const WstringConverterPtr& = 0, ConversionFlags = lenientConversion);

//
// Converts the given narrow string to a wide string
//
// If the StringConverter parameter is null, the input's narrow string 
// encoding is UTF-8.  
// If the WstringConverter parameter is null, the result's wstring 
// encoding is UTF-16 or UTF-32 depending on the size of wchar_t.
//
ICE_UTIL_API std::wstring
stringToWstring(const std::string&, const StringConverterPtr& = 0, 
                const WstringConverterPtr& = 0, ConversionFlags = lenientConversion);


//
// Converts the given string from the native narrow string encoding to
// UTF-8 using the given converter. If the converter is null, returns
// the given string.
//
ICE_UTIL_API std::string
nativeToUTF8(const std::string&, const StringConverterPtr&);

//
// Converts the given string from UTF-8 to the native narrow string
// encoding using the given converter. If the converter is null,
// returns the given string.
//
ICE_UTIL_API std::string
UTF8ToNative(const std::string&, const StringConverterPtr&);

}

#endif
