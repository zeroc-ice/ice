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
#include <IceUtil/Unicode.h>

#include <string>

namespace IceUtil
{

//
// Raised by string converters when an encoding converseion fails.
//
class ICE_UTIL_API IllegalConversionException : public ::IceUtil::Exception
{
public:

    IllegalConversionException(const char*, int);
    IllegalConversionException(const char*, int, const ::std::string&);
    virtual ~IllegalConversionException() throw();

    virtual ::std::string ice_name() const;
    virtual void ice_print(::std::ostream&) const;
    virtual IllegalConversionException* ice_clone() const;
    virtual void ice_throw() const;

    std::string reason() const;
    
private:

    std::string _reason;
};

//
// Provides bytes to toUTF8. Raises MemoryLimitException when too many
// bytes are requested.
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
// It report errors by raising IllegalConversionException or MemoryLimitException.
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
// Retrive the per process narrow string converter. Access to the 
// converter is protected by a static mutex.
//
ICE_UTIL_API StringConverterPtr getProcessStringConverter();

//
// Set the per process narrow string converter. Access to the 
// converter is protected by a static mutex.
//
ICE_UTIL_API void setProcessStringConverter(const StringConverterPtr&);

//
// Retrive the per process wide string converter. Access to the 
// converter is protected by a static mutex.
//
ICE_UTIL_API WstringConverterPtr getProcessWstringConverter();

//
// Set the per process wide string converter. Access to the 
// converter is protected by a static mutex.
//
ICE_UTIL_API void setProcessWstringConverter(const WstringConverterPtr&);

//
// Convert the given wide string from the native wide string encoding to a
// narrow string with the native narrow string encoding.
//
// The StringConverter param can be null in that case the default narrow
// string encoding is assumed to be UTF8.
//
// The WstringConverter param can be null in that case the default wide
// string encoding is assumed, that would be UTF16 or UTF32 depending of
// the platform.
//
ICE_UTIL_API std::string
wnativeToNative(const StringConverterPtr&, const WstringConverterPtr&, const std::wstring&);

//
// Convert the given narrow string from the native narrow string encoding
// to a wide string with the native wide string encoding.
//
// The StringConverter param can be null in that case the default narrow
// string encoding is assumed to be UTF8.
//
// The WstringConverter param can be null in that case the default wide
// string encoding is assumed, that would be UTF16 or UTF32 depending of
// the platform.
//
ICE_UTIL_API std::wstring
nativeToWnative(const StringConverterPtr&, const WstringConverterPtr&, const std::string&);

//
// Converts the given string from the native narrow string encoding to
// UTF8 using the given converter. If the converter is null, returns
// the given string.
//
ICE_UTIL_API std::string
nativeToUTF8(const StringConverterPtr&, const std::string&);

//
// Converts the given string from UTF8 to the native narrow string
// encoding using the given converter. If the converter is null,
// returns the given string.
//
ICE_UTIL_API std::string
UTF8ToNative(const StringConverterPtr&, const std::string&);

}

#endif
