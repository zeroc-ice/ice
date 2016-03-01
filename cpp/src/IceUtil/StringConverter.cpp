// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/StringConverter.h>
#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/ScopedArray.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/Unicode.h>

using namespace IceUtil;
using namespace IceUtilInternal;
using namespace std;

namespace
{

IceUtil::Mutex* processStringConverterMutex = 0;
IceUtil::StringConverterPtr processStringConverter;
IceUtil::WstringConverterPtr processWstringConverter;

class Init
{
public:

    Init()
    {
        processStringConverterMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete processStringConverterMutex;
        processStringConverterMutex = 0;
    }
};

Init init;

}

namespace
{

class UTF8BufferI : public UTF8Buffer
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

    Byte* getMoreBytes(size_t howMany, Byte* firstUnused)
    {
        if(_buffer == 0)
        {
            _buffer = static_cast<Byte*>(malloc(howMany));
        }
        else
        {
            assert(firstUnused != 0);
            _offset = firstUnused - _buffer;
            _buffer = static_cast<Byte*>(realloc(_buffer, _offset + howMany));
        }
        
        if(!_buffer)
        {
            throw std::bad_alloc();
        }
        return _buffer + _offset;
    }

    Byte* getBuffer()
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

    IceUtil::Byte* _buffer;
    size_t _offset;
};

}



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
            throw IceUtil::IllegalConversionException(__FILE__, __LINE__, "wide string source exhausted");
        case sourceIllegal:
            throw IceUtil::IllegalConversionException(__FILE__, __LINE__, "wide string source illegal");
        default:
        {
            assert(0);
            throw IceUtil::IllegalConversionException(__FILE__, __LINE__);
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
            throw IceUtil::IllegalConversionException(__FILE__, __LINE__, "UTF-8 string source exhausted");
        case sourceIllegal:
            throw IceUtil::IllegalConversionException(__FILE__, __LINE__, "UTF-8 string source illegal");
        default:
        {
            assert(0);
            throw IceUtil::IllegalConversionException(__FILE__, __LINE__);
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
    
    //
    // The following code pages doesn't support MB_ERR_INVALID_CHARS flag
    // see http://msdn.microsoft.com/en-us/library/windows/desktop/dd319072(v=vs.85).aspx
    //
    DWORD flags =
        (_cp == 50220 || _cp == 50221 || _cp == 50222 ||
         _cp == 50225 || _cp == 50227 || _cp == 50229 || 
         _cp == 65000 || _cp == 42 || (_cp >= 57002 && _cp <= 57011)) ? 0 : MB_ERR_INVALID_CHARS;
    
    do
    {
        size = size == 0 ? sourceSize + 2 : 2 * size;
        wbuffer.reset(new wchar_t[size]);

        writtenWchar = MultiByteToWideChar(_cp, flags, sourceStart,
                                           sourceSize, wbuffer.get(), size);
    } while(writtenWchar == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

    if(writtenWchar == 0)
    {
        throw IllegalConversionException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
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

    if(_cp == CP_UTF8)
    {
        string tmp(reinterpret_cast<const char*>(sourceStart), sourceEnd - sourceStart);
        tmp.swap(target);
        return;
    }

    //
    // First convert to wstring (UTF-16)
    //
    wstring wtarget;
    _unicodeWstringConverter.fromUTF8(sourceStart, sourceEnd, wtarget);

    //
    // WC_ERR_INVALID_CHARS conversion flag is only supported with 65001 (UTF-8) and
    // 54936 (GB18030 Simplified Chinese)
    //
    DWORD flags = (_cp == 65001 || _cp == 54936) ? WC_ERR_INVALID_CHARS : 0;
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
        writtenChar = WideCharToMultiByte(_cp, flags, wtarget.data(), static_cast<int>(wtarget.size()),
                                          buffer.get(), size, 0, 0);
    } while(writtenChar == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

    if(writtenChar == 0)
    {
        throw IllegalConversionException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
    }

    target.assign(buffer.get(), writtenChar);
}

#endif


StringConverterPtr
IceUtil::getProcessStringConverter()
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(processStringConverterMutex);
    return processStringConverter;
}

void
IceUtil::setProcessStringConverter(const StringConverterPtr& converter)
{
   IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(processStringConverterMutex);
   processStringConverter = converter;
}

WstringConverterPtr
IceUtil::getProcessWstringConverter()
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(processStringConverterMutex);
    return processWstringConverter;
}

void
IceUtil::setProcessWstringConverter(const WstringConverterPtr& converter)
{
   IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(processStringConverterMutex);
   processWstringConverter = converter;
}

string
IceUtil::wstringToString(const wstring& v, const StringConverterPtr& converter, const WstringConverterPtr& wConverter,
                         IceUtil::ConversionFlags flags)
{
    string target;
    if(!v.empty())
    {
        //
        // First convert to UTF8 narrow string.
        //
        if(wConverter)
        {
            UTF8BufferI buffer;
            Byte* last = wConverter->toUTF8(v.data(), v.data() + v.size(), buffer);
            target = string(reinterpret_cast<const char*>(buffer.getBuffer()), last - buffer.getBuffer());
        }
        else
        {
            size_t size = v.size() * 4 * sizeof(char);

            Byte* outBuf = new Byte[size];
            Byte* targetStart = outBuf; 
            Byte* targetEnd = outBuf + size;

            const wchar_t* sourceStart = v.data();
  
            ConversionResult cr = 
                convertUTFWstringToUTF8(
                    sourceStart, sourceStart + v.size(), 
                    targetStart, targetEnd, flags);
                
            if(cr != conversionOK)
            {
                delete[] outBuf;
                assert(cr == sourceExhausted || cr == sourceIllegal);
                throw IllegalConversionException(__FILE__, __LINE__, 
                                                 cr == sourceExhausted ? "partial character" : "bad encoding");
            }
            
            target = string(reinterpret_cast<char*>(outBuf), static_cast<size_t>(targetStart - outBuf));
            delete[] outBuf;
        }

        //
        // If narrow string converter is present convert to the native narrow string encoding, otherwise 
        // native narrow string encoding is UTF8 and we are done.
        //
        if(converter)
        {
            string tmp;
            converter->fromUTF8(reinterpret_cast<const Byte*>(target.data()), 
                                reinterpret_cast<const Byte*>(target.data() + target.size()), tmp);
            tmp.swap(target);
        }
    }
    return target;
}

wstring
IceUtil::stringToWstring(const string& v, const StringConverterPtr& converter, 
                         const WstringConverterPtr& wConverter, IceUtil::ConversionFlags flags)
{
    wstring target;
    if(!v.empty())
    {
        //
        // If there is a narrow string converter use it to convert to UTF8, otherwise the narrow
        // string is already UTF8 encoded.
        //
        string tmp;
        if(converter)
        {
            UTF8BufferI buffer;
            Byte* last = converter->toUTF8(v.data(), v.data() + v.size(), buffer);
            tmp = string(reinterpret_cast<const char*>(buffer.getBuffer()), last - buffer.getBuffer());
        }
        else
        {
            tmp = v;
        }

        //
        // If there is a wide string converter use fromUTF8 to convert to the wide string native encoding.
        //
        if(wConverter)
        {
            wConverter->fromUTF8(reinterpret_cast<const Byte*>(tmp.data()), 
                                 reinterpret_cast<const Byte*>(tmp.data() + tmp.size()), target);
        }
        else
        {
            const Byte* sourceStart = reinterpret_cast<const Byte*>(tmp.data());
            
            ConversionResult cr = 
                convertUTF8ToUTFWstring(sourceStart, sourceStart + tmp.size(), target, flags);

            if(cr != conversionOK)
            {
                assert(cr == sourceExhausted || cr == sourceIllegal);

                throw IllegalConversionException(__FILE__, __LINE__,
                                                 cr == sourceExhausted ? "partial character" : "bad encoding");
            }
        }
    }
    return target;
}

string
IceUtil::nativeToUTF8(const string& str, const IceUtil::StringConverterPtr& converter)
{
    if(!converter || str.empty())
    {
        return str;
    }    
    UTF8BufferI buffer;
    Byte* last = converter->toUTF8(str.data(), str.data() + str.size(), buffer);
    return string(reinterpret_cast<const char*>(buffer.getBuffer()), last - buffer.getBuffer());
}

string
IceUtil::UTF8ToNative(const string& str, const IceUtil::StringConverterPtr& converter)
{
    if(!converter || str.empty())
    {
        return str;
    }
    string tmp;
    converter->fromUTF8(reinterpret_cast<const Byte*>(str.data()),
                        reinterpret_cast<const Byte*>(str.data() + str.size()), tmp);
    return tmp;
}
