// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/StringConverter.h>
#include <IceUtil/IceUtil.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/ScopedArray.h>
#include <Ice/Initialize.h>
#include <Ice/Instance.h>
#include <Ice/LocalException.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Communicator.h>

#ifndef _WIN32
#include <Ice/IconvStringConverter.h>
#endif

#ifdef __MINGW32__
#  include <limits.h>
#endif

using namespace IceUtil;
using namespace IceUtilInternal;
using namespace std;

namespace
{

class UTF8BufferI : public Ice::UTF8Buffer
{
public:

   UTF8BufferI() :
        _buffer(0),
        _offset(0)
    {
    }
    
   ~UTF8BufferI()
    {
        free(_buffer);
    }

    Ice::Byte* getMoreBytes(size_t howMany, Byte* firstUnused)
    {
        if(_buffer == 0)
        {
            _buffer = (Byte*)malloc(howMany);
        }
        else
        {
            assert(firstUnused != 0);
            _offset = firstUnused - _buffer;
            _buffer = (Byte*)realloc(_buffer, _offset + howMany);
        }
        
        if(!_buffer)
        {
            throw std::bad_alloc();
        }
        return _buffer + _offset;
    }
    
    Ice::Byte* getBuffer()
    {
        return _buffer;
    }
    
    void reset()
    {
        free(_buffer);
        _buffer = 0;
        _offset = 0;
    }

private:

    Ice::Byte* _buffer;
    size_t _offset;
};
}



namespace Ice
{

UnicodeWstringConverter::UnicodeWstringConverter(ConversionFlags flags) :
    _conversionFlags(flags)
{
}

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
                                  targetStart, targetEnd, _conversionFlags))
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
                                  wstring& target) const
{
    if(sourceStart == sourceEnd)
    {
        target = L"";
        return;
    }

    ConversionResult result = 
        convertUTF8ToUTFWstring(sourceStart, sourceEnd, target, _conversionFlags);

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

#ifdef _WIN32
WindowsStringConverter::WindowsStringConverter(unsigned int cp) :
    _cp(cp)
{
}

Byte*
WindowsStringConverter::toUTF8(const char* sourceStart,
                               const char* sourceEnd,
                               UTF8Buffer& buffer) const
{
    //
    // First convert to UTF-16
    //
    int sourceSize = static_cast<int>(sourceEnd - sourceStart);
    if(sourceSize == 0)
    {
        return buffer.getMoreBytes(1, 0);
    }

    int size = 0;
    int writtenWchar = 0;
    ScopedArray<wchar_t> wbuffer;
    do
    {
        size = size == 0 ? sourceSize + 2 : 2 * size;
        wbuffer.reset(new wchar_t[size]);

        writtenWchar = MultiByteToWideChar(_cp, MB_ERR_INVALID_CHARS, sourceStart,
                                           sourceSize, wbuffer.get(), size);
    } while(writtenWchar == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

    if(writtenWchar == 0)
    {
        throw StringConversionException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
    }

    //
    // Then convert this UTF-16 wbuffer into UTF-8
    //
    return _unicodeWstringConverter.toUTF8(wbuffer.get(), wbuffer.get() + writtenWchar, buffer);
}

void
WindowsStringConverter::fromUTF8(const Byte* sourceStart, const Byte* sourceEnd,
                                 string& target) const
{
    if(sourceStart == sourceEnd)
    {
        target = "";
        return;
    }

    //
    // First convert to wstring (UTF-16)
    //
    wstring wtarget;
    _unicodeWstringConverter.fromUTF8(sourceStart, sourceEnd, wtarget);

    //
    // And then to a multi-byte narrow string
    //
    int size = 0;
    int writtenChar = 0;
    ScopedArray<char> buffer;
    do
    {
        size = size == 0 ? static_cast<int>(sourceEnd - sourceStart) + 2 : 2 * size;
        buffer.reset(new char[size]);
        writtenChar = WideCharToMultiByte(_cp, 0, wtarget.data(), static_cast<int>(wtarget.size()),
                                          buffer.get(), size, 0, 0);
    } while(writtenChar == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

    if(writtenChar == 0)
    {
        throw StringConversionException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
    }

    target.assign(buffer.get(), writtenChar);
}

#endif

StringConverterPlugin::StringConverterPlugin(const CommunicatorPtr& communicator,
                                             const StringConverterPtr& stringConverter, 
                                             const WstringConverterPtr& wstringConverter)
{
    if(communicator == 0)
    {
        throw PluginInitializationException(__FILE__, __LINE__, "Communicator cannot be null");
    }
    
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);

    if(stringConverter != 0)
    {
        instance->setStringConverter(stringConverter);
    }
    if(wstringConverter != 0)
    {
        instance->setWstringConverter(wstringConverter);
    }
}

void
StringConverterPlugin::initialize()
{
}

void
StringConverterPlugin::destroy()
{
}

}

//
// The entry point for the "string converter" plug-in built-in the Ice library
//
extern "C"
{

using namespace Ice;

ICE_DECLSPEC_EXPORT Plugin*
createStringConverter(const CommunicatorPtr& communicator, const string& name, const StringSeq& args)
{
    StringConverterPtr stringConverter;
    WstringConverterPtr wstringConverter;

    if(args.size() > 2)
    {
        Error out(communicator->getLogger());
        out << "Plugin " << name << ": too many arguments";
        return 0;
    }

    try
    {

#ifdef _WIN32
    
        int cp = -1;

        for(size_t i = 0; i < args.size(); ++i)
        {
            if(args[i].find("windows=") == 0)
            {
                cp = atoi(args[i].substr(strlen("windows=")).c_str());
            }
            else if(args[i].find("iconv=") != 0)
            {
                Error out(communicator->getLogger());
                out << "Plugin " << name << ": invalid \"" << args[i] << "\" argument";
                return 0;
            }
        }
        
        if(cp == -1)
        {
            Error out(communicator->getLogger());
            out << "Plugin " << name << ": missing windows=<code page> argument";
            return 0;
        }

        if(cp == 0 || cp == INT_MAX || cp < 0)
        {
            Error out(communicator->getLogger());
            out << "Plugin " << name << ": invalid Windows code page";
            return 0;
        }

        stringConverter = new WindowsStringConverter(static_cast<unsigned int>(cp));
#else
        StringSeq iconvArgs;

        for(size_t i = 0; i < args.size(); ++i)
        {
            if(args[i].find("iconv=") == 0)
            {
                if(!IceUtilInternal::splitString(args[i].substr(strlen("iconv=")), ", \t\r\n", iconvArgs))
                {
                    Error out(communicator->getLogger());
                    out << "Plugin " << name << ": invalid iconv argument";
                    return 0;
                }
            }
            else if(args[i].find("windows=") != 0)
            {
                Error out(communicator->getLogger());
                out << "Plugin " << name << ": invalid \"" << args[i] << "\" argument";
                return 0;
            }
        }
        
        switch(iconvArgs.size())
        {
            case 0:
            {
                stringConverter = new IconvStringConverter<char>;
                break;
            }
            case 1:
            {
                stringConverter = new IconvStringConverter<char>(iconvArgs[0].c_str());
                break;
            }
            case 2:
            {
                stringConverter = new IconvStringConverter<char>(iconvArgs[0].c_str());
                wstringConverter = new IconvStringConverter<wchar_t>(iconvArgs[1].c_str());
                break;
            }
            default:
            {
                assert(0);
            }
        }

#endif    

        return new StringConverterPlugin(communicator, stringConverter, wstringConverter);
    }
    catch(const std::exception& ex)
    {
        Error out(communicator->getLogger());
        out << "Plugin " << name << ": creation failed with " << ex.what();
        return 0;
    }
    catch(...)
    {
        Error out(communicator->getLogger());
        out << "Plugin " << name << ": creation failed with unknown exception";
        return 0;
    }
}
}

string
Ice::nativeToUTF8(const Ice::StringConverterPtr& converter, const string& str)
{
    if(!converter)
    {
        return str;
    }
    if(str.empty())
    {
        return str;
    }
    UTF8BufferI buffer;
    Ice::Byte* last = converter->toUTF8(str.data(), str.data() + str.size(), buffer);
    return string(reinterpret_cast<const char*>(buffer.getBuffer()), last - buffer.getBuffer());
}

string
Ice::nativeToUTF8(const Ice::CommunicatorPtr& ic, const string& str)
{
    return nativeToUTF8(IceInternal::getInstance(ic)->initializationData().stringConverter, str);
}

string
Ice::UTF8ToNative(const Ice::StringConverterPtr& converter, const string& str)
{
    if(!converter)
    {
        return str;
    }
    if(str.empty())
    {
        return str;
    }
    string tmp;
    converter->fromUTF8(reinterpret_cast<const Ice::Byte*>(str.data()),
                        reinterpret_cast<const Ice::Byte*>(str.data() + str.size()), tmp);
    return tmp;
}

string
Ice::UTF8ToNative(const Ice::CommunicatorPtr& ic, const std::string& str)
{
    return UTF8ToNative(IceInternal::getInstance(ic)->initializationData().stringConverter, str);
}
