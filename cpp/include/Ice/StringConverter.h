// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STRING_CONVERTER_H
#define ICE_STRING_CONVERTER_H

#include <Ice/Config.h>
#include <Ice/CommunicatorF.h>
#include <Ice/Plugin.h>
#include <IceUtil/Exception.h>
#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <IceUtil/Unicode.h>

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

    UnicodeWstringConverter(IceUtil::ConversionFlags = IceUtil::lenientConversion);

    virtual Byte* toUTF8(const wchar_t*, const wchar_t*, UTF8Buffer&) const;

    virtual void fromUTF8(const Byte*, const Byte*, std::wstring&) const;

private:
    const IceUtil::ConversionFlags _conversionFlags;
};

#ifdef _WIN32

//
// Converts to/from UTF-8 using MultiByteToWideChar and WideCharToMultiByte
//

class ICE_API WindowsStringConverter : public StringConverter
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
// A special plug-in that sets stringConverter and wstringConverter during
// construction (when the provided stringConverter resp. wstringConverter
// are not null). Both initialize and destroy are no-op. See Ice::InitializationData.
//

class ICE_API StringConverterPlugin : public Ice::Plugin
{
public:

    StringConverterPlugin(const CommunicatorPtr& communicator, 
                          const StringConverterPtr&, const WstringConverterPtr& = 0);

    virtual void initialize();

    virtual void destroy();
};

//
// Converts the given string from the native narrow string encoding to
// UTF8 using the given converter. If the converter is null, returns
// the given string.
//
ICE_API std::string
nativeToUTF8(const Ice::StringConverterPtr&, const std::string&);

//
// Converts the given string from the native narrow string encoding to
// UTF8 using the communicator's converter. If the converter is null,
// returns the given string.
//
ICE_API std::string
nativeToUTF8(const Ice::CommunicatorPtr&, const std::string&);

//
// Converts the given string from UTF8 to the native narrow string
// encoding using the given converter. If the converter is null,
// returns the given string.
//
ICE_API std::string
UTF8ToNative(const Ice::StringConverterPtr&, const std::string&);

//
// Converts the given string from UTF8 to the native narrow string
// encoding using the communicator's converter. If the converter is
// null, returns the given string.
//
ICE_API std::string
UTF8ToNative(const Ice::CommunicatorPtr&, const std::string&);

}

#endif
