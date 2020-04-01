//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/StringConverter.h>
#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/StringUtil.h>

#ifdef ICE_HAS_CODECVT_UTF8
#include <codecvt>
#include <locale>
#else
#include <IceUtil/Unicode.h>
#endif

using namespace IceUtil;
using namespace IceUtilInternal;
using namespace std;

namespace
{

IceUtil::Mutex* processStringConverterMutex = 0;
IceUtil::StringConverterPtr processStringConverter;
IceUtil::WstringConverterPtr processWstringConverter;

#ifdef ICE_HAS_CODECVT_UTF8

template<size_t wcharSize>
struct SelectCodeCvt;

template<>
struct SelectCodeCvt<2>
{
    typedef std::codecvt_utf8_utf16<wchar_t> Type;
};

template<>
struct SelectCodeCvt<4>
{
    typedef std::codecvt_utf8<wchar_t> Type;
};

class UnicodeWstringConverter : public WstringConverter
{
public:

    virtual Byte* toUTF8(const wchar_t* sourceStart, const wchar_t* sourceEnd, UTF8Buffer& buffer) const
    {
        //
        // Max bytes for a character encoding in UTF-8 is 4,
        // however MSVC returns 6
        //
#ifdef _MSC_VER
        assert(_codecvt.max_length() == 4 || _codecvt.max_length() == 6);
#else
        assert(_codecvt.max_length() == 4);
#endif
        if(sourceStart == sourceEnd)
        {
            return buffer.getMoreBytes(1, 0);
        }

        char* targetStart = 0;
        char* targetEnd = 0;
        char* targetNext = 0;

        mbstate_t state = mbstate_t(); // must be initialized!
        const wchar_t* sourceNext = sourceStart;

        bool more = false;

        //
        // The number of bytes we request from buffer for each remaining source character
        //
        size_t factor = 2;

        do
        {
            assert(factor <= 4);
            const size_t chunkSize = std::max<size_t>(static_cast<size_t>(sourceEnd - sourceStart) * factor, 4);
            ++factor; // at the next round, we'll allocate more bytes per remaining source character

            targetStart = reinterpret_cast<char*>(buffer.getMoreBytes(chunkSize, reinterpret_cast<Byte*>(targetNext)));
            targetEnd = targetStart + chunkSize;
            targetNext = targetStart;

            codecvt_base::result result =
                _codecvt.out(state, sourceStart, sourceEnd, sourceNext, targetStart, targetEnd, targetNext);

            switch(result)
            {
                case codecvt_base::ok:
                    //
                    // MSVC returns ok when target is exhausted
                    //
                    more = sourceNext < sourceEnd;
                    break;

                case codecvt_base::partial:
                    //
                    // clang/libc++ and g++5 return partial when target is exhausted
                    //
                    more = true;
                    assert(sourceNext < sourceEnd);
                    break;

                case codecvt_base::noconv:
                    //
                    // Unexpected
                    //
                    assert(0);
                    throw IllegalConversionException(__FILE__, __LINE__, "codecvt.out noconv");

                default:
                    throw IllegalConversionException(__FILE__, __LINE__, "codecvt.out error");
            }

            if(targetStart == targetNext)
            {
                // We didn't convert a single character
                throw IllegalConversionException(__FILE__, __LINE__,
                                                 "no character converted by codecvt.out");
            }

            sourceStart = sourceNext;
        } while (more);

        return reinterpret_cast<Byte*>(targetNext);
    }

    virtual void fromUTF8(const Byte* sourceStart, const Byte* sourceEnd, wstring& target) const
    {
        const size_t sourceSize = static_cast<size_t>(sourceEnd - sourceStart);

        if(sourceSize == 0)
        {
            target = L"";
        }
        else
        {
            target.resize(sourceSize);
            wchar_t* targetStart = const_cast<wchar_t*>(target.data());
            wchar_t* targetEnd = targetStart + sourceSize;
            wchar_t* targetNext = targetStart;

            const char* sourceNext = reinterpret_cast<const char*>(sourceStart);

            mbstate_t state = mbstate_t();

            codecvt_base::result result = _codecvt.in(state,
                                                      reinterpret_cast<const char*>(sourceStart),
                                                      reinterpret_cast<const char*>(sourceEnd),
                                                      sourceNext,
                                                      targetStart, targetEnd, targetNext);

            if(result != codecvt_base::ok)
            {
                throw IllegalConversionException(__FILE__, __LINE__, "codecvt.in failure");
            }

            target.resize(static_cast<size_t>(targetNext - targetStart));
        }
    }

private:

    typedef SelectCodeCvt<sizeof(wchar_t)>::Type CodeCvt;
    const CodeCvt _codecvt;
};

#else

class UnicodeWstringConverter : public WstringConverter
{
public:

    virtual Byte* toUTF8(const wchar_t* sourceStart, const wchar_t* sourceEnd, UTF8Buffer& buffer) const
    {
        if(sourceStart == sourceEnd)
        {
            return buffer.getMoreBytes(1, 0);
        }

        Byte* targetStart = 0;
        Byte* targetEnd = 0;

        //
        // The number of bytes we request from buffer for each remaining source character
        //
        size_t factor = 2;

        do
        {
            assert(factor <= 4);
            const size_t chunkSize = std::max<size_t>((sourceEnd - sourceStart) * factor, 4);
            ++factor; // at the next round, we'll allocate more bytes per remaining source character

            targetStart = buffer.getMoreBytes(chunkSize, targetStart);
            targetEnd = targetStart + chunkSize;
        }
        while(convertUTFWstringToUTF8(sourceStart, sourceEnd, targetStart, targetEnd) == false);

        return targetStart;
    }

    virtual void fromUTF8(const Byte* sourceStart, const Byte* sourceEnd, wstring& target) const
    {
        if(sourceStart == sourceEnd)
        {
            target = L"";
        }
        else
        {
            convertUTF8ToUTFWstring(sourceStart, sourceEnd, target);
        }
    }
};

#endif

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

const WstringConverterPtr&
getUnicodeWstringConverter()
{
    static const WstringConverterPtr unicodeWstringConverter = std::make_shared<UnicodeWstringConverter>();
    return unicodeWstringConverter;
}

class UTF8BufferI : public UTF8Buffer
{
public:

    //
    // Returns the first unused byte in the resized buffer
    //
    Byte* getMoreBytes(size_t howMany, Byte* firstUnused)
    {
        size_t bytesUsed = 0;
        if(firstUnused != 0)
        {
            bytesUsed = static_cast<size_t>(firstUnused - reinterpret_cast<const Byte*>(_buffer.data()));
        }

        if(_buffer.size() < howMany + bytesUsed)
        {
            _buffer.resize(bytesUsed + howMany);
        }

        return const_cast<Byte*>(reinterpret_cast<const Byte*>(_buffer.data())) + bytesUsed;
    }

    void swap(string& other, const Byte* tail)
    {
        assert(tail >= reinterpret_cast<const Byte*>(_buffer.data()));
        _buffer.resize(static_cast<size_t>(tail - reinterpret_cast<const Byte*>(_buffer.data())));
        other.swap(_buffer);
    }

private:
    string _buffer;
};

}

IceUtil::UTF8Buffer::~UTF8Buffer()
{
    // Out of line to avoid weak vtable
}

WstringConverterPtr
IceUtil::createUnicodeWstringConverter()
{
    return getUnicodeWstringConverter();
}

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
    if(processWstringConverter)
    {
        return processWstringConverter;
    }
    else
    {
        return getUnicodeWstringConverter();
    }
}

void
IceUtil::setProcessWstringConverter(const WstringConverterPtr& converter)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(processStringConverterMutex);
    processWstringConverter = converter;
}

string
IceUtil::wstringToString(const wstring& v, const StringConverterPtr& converter, const WstringConverterPtr& wConverter)
{
    string target;
    if(!v.empty())
    {
        const WstringConverterPtr& wConverterWithDefault = wConverter ? wConverter : getUnicodeWstringConverter();

        //
        // First convert to UTF-8 narrow string.
        //
        UTF8BufferI buffer;
        Byte* last = wConverterWithDefault->toUTF8(v.data(), v.data() + v.size(), buffer);
        buffer.swap(target, last);

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
IceUtil::stringToWstring(const string& v, const StringConverterPtr& converter, const WstringConverterPtr& wConverter)
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
            buffer.swap(tmp, last);
        }
        else
        {
            tmp = v;
        }

        const WstringConverterPtr& wConverterWithDefault = wConverter ? wConverter : getUnicodeWstringConverter();

        //
        // Convert from UTF-8 to the wide string encoding
        //
        wConverterWithDefault->fromUTF8(reinterpret_cast<const Byte*>(tmp.data()),
                                        reinterpret_cast<const Byte*>(tmp.data() + tmp.size()), target);

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
    string result;
    buffer.swap(result, last);
    return result;
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

#ifdef ICE_HAS_CODECVT_UTF8
#   if defined(_MSC_VER) && (_MSC_VER >= 1910 && _MSC_VER < 1920)
//
// Workaround for compiler bug - see http://stackoverflow.com/questions/32055357
//
typedef unsigned short Char16T;
typedef unsigned int Char32T;
#   else
typedef char16_t Char16T;
typedef char32_t Char32T;
#   endif
#endif

vector<unsigned short>
IceUtilInternal::toUTF16(const vector<Byte>& source)
{
    vector<unsigned short> result;
    if(!source.empty())
    {

#ifdef ICE_HAS_CODECVT_UTF8
    assert(sizeof(Char16T) == sizeof(unsigned short));

    typedef wstring_convert<codecvt_utf8_utf16<Char16T>, Char16T> Convert;

    Convert convert;

    try
    {
        Convert::wide_string ws = convert.from_bytes(reinterpret_cast<const char*>(&source.front()),
                                                     reinterpret_cast<const char*>(&source.front() + source.size()));

        result = vector<unsigned short>(reinterpret_cast<const unsigned short*>(ws.data()),
                                        reinterpret_cast<const unsigned short*>(ws.data()) + ws.length());
    }
    catch(const std::range_error& ex)
    {
        throw IllegalConversionException(__FILE__, __LINE__, ex.what());
    }

#else
    convertUTF8ToUTF16(source, result);
#endif
    }
    return result;
}

vector<unsigned int>
IceUtilInternal::toUTF32(const vector<Byte>& source)
{
    vector<unsigned int> result;
    if(!source.empty())
    {

#ifdef ICE_HAS_CODECVT_UTF8
    assert(sizeof(Char32T) == sizeof(unsigned int));

    typedef wstring_convert<codecvt_utf8<Char32T>, Char32T> Convert;
    Convert convert;

    try
    {
        Convert::wide_string ws = convert.from_bytes(reinterpret_cast<const char*>(&source.front()),
                                                     reinterpret_cast<const char*>(&source.front() + source.size()));

        result = vector<unsigned int>(reinterpret_cast<const unsigned int*>(ws.data()),
                                      reinterpret_cast<const unsigned int*>(ws.data()) + ws.length());
    }
    catch(const std::range_error& ex)
    {
        throw IllegalConversionException(__FILE__, __LINE__, ex.what());
    }

#else
    convertUTF8ToUTF32(source, result);
#endif
    }
    return result;
}

vector<Byte>
IceUtilInternal::fromUTF32(const vector<unsigned int>& source)
{
    vector<Byte> result;
    if(!source.empty())
    {

#ifdef ICE_HAS_CODECVT_UTF8
    assert(sizeof(Char32T) == sizeof(unsigned int));

    typedef wstring_convert<codecvt_utf8<Char32T>, Char32T> Convert;
    Convert convert;

    try
    {
        Convert::byte_string bs = convert.to_bytes(reinterpret_cast<const Char32T*>(&source.front()),
                                                   reinterpret_cast<const Char32T*>(&source.front() + source.size()));

        result = vector<Byte>(reinterpret_cast<const Byte*>(bs.data()),
                              reinterpret_cast<const Byte*>(bs.data()) + bs.length());
        }
    catch(const std::range_error& ex)
    {
        throw IllegalConversionException(__FILE__, __LINE__, ex.what());
    }

#else
    convertUTF32ToUTF8(source, result);
#endif
    }
    return result;
}

#ifdef _WIN32

namespace
{
//
// Converts to/from UTF-8 using MultiByteToWideChar and WideCharToMultiByte
//
class WindowsStringConverter : public StringConverter
{
public:

    explicit WindowsStringConverter(unsigned int);

    virtual Byte* toUTF8(const char*, const char*, UTF8Buffer&) const;

    virtual void fromUTF8(const Byte*, const Byte*, string& target) const;

private:
    unsigned int _cp;
};

WindowsStringConverter::WindowsStringConverter(unsigned int cp) :
    _cp(cp)
{
}

Byte*
WindowsStringConverter::toUTF8(const char* sourceStart, const char* sourceEnd, UTF8Buffer& buffer) const
{
    //
    // First convert to UTF-16
    //
    int sourceSize = static_cast<int>(sourceEnd - sourceStart);
    if(sourceSize == 0)
    {
        return buffer.getMoreBytes(1, 0);
    }

    int writtenWchar = 0;
    wstring wbuffer;

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
        wbuffer.resize(wbuffer.size() == 0 ? sourceSize + 2 : 2 * wbuffer.size());
        writtenWchar = MultiByteToWideChar(_cp, flags, sourceStart, sourceSize,
                                           const_cast<wchar_t*>(wbuffer.data()), static_cast<int>(wbuffer.size()));
    } while(writtenWchar == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

    if(writtenWchar == 0)
    {
        throw IllegalConversionException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
    }

    wbuffer.resize(static_cast<size_t>(writtenWchar));

    //
    // Then convert this UTF-16 wbuffer into UTF-8
    //
    return getUnicodeWstringConverter()->toUTF8(wbuffer.data(), wbuffer.data() + wbuffer.size(), buffer);
}

void
WindowsStringConverter::fromUTF8(const Byte* sourceStart, const Byte* sourceEnd, string& target) const
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
    getUnicodeWstringConverter()->fromUTF8(sourceStart, sourceEnd, wtarget);

    //
    // WC_ERR_INVALID_CHARS conversion flag is only supported with 65001 (UTF-8) and
    // 54936 (GB18030 Simplified Chinese)
    //
    DWORD flags = (_cp == 65001 || _cp == 54936) ? WC_ERR_INVALID_CHARS : 0;

    //
    // And then to a multi-byte narrow string
    //
    int writtenChar = -1;
    do
    {
        target.resize(writtenChar == -1 ?
                      std::max<size_t>(sourceEnd - sourceStart + 2, target.size()) :
                      2 * target.size());

        writtenChar = WideCharToMultiByte(_cp, flags, wtarget.data(), static_cast<int>(wtarget.size()),
                                          const_cast<char*>(target.data()), static_cast<int>(target.size()),
                                          0, 0);
    } while(writtenChar == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

    if(writtenChar == 0)
    {
        throw IllegalConversionException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
    }

    target.resize(static_cast<size_t>(writtenChar));
}
}

StringConverterPtr
IceUtil::createWindowsStringConverter(unsigned int cp)
{
    return std::make_shared<WindowsStringConverter>(cp);
}
#endif
