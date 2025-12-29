// Copyright (c) ZeroC, Inc.

// TODO codecvt was deprecated in C++17 and cause build failures in C++17 mode
// For VC++ we should replace this code with MultiByteToWideChar() and WideCharToMultiByte()
#if defined(_MSC_VER) && (_MSVC_LANG >= 201703L)
#    define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#elif (__cplusplus >= 201703L)
#    include "DisableWarnings.h"
#    if defined(__clang__)                                   // for clang in C++26 mode
#        define _LIBCPP_ENABLE_CXX26_REMOVED_CODECVT         // NOLINT(cert-dcl37-c,cert-dcl51-cpp)
#        define _LIBCPP_ENABLE_CXX26_REMOVED_WSTRING_CONVERT // NOLINT(cert-dcl37-c,cert-dcl51-cpp)
#    endif
#endif

#include "Ice/StringConverter.h"

#ifdef _WIN32
#    include "Ice/StringUtil.h"

#    include <windows.h>
#endif

#include <cassert>
#include <codecvt>
#include <locale>
#include <mutex>

using namespace Ice;
using namespace IceInternal;
using namespace std;

namespace
{
    mutex processStringConverterMutex;
    StringConverterPtr processStringConverter;
    WstringConverterPtr processWstringConverter;

    template<size_t wcharSize> struct SelectCodeCvt;

    template<> struct SelectCodeCvt<2>
    {
        using Type = std::codecvt_utf8_utf16<wchar_t>;
    };

    template<> struct SelectCodeCvt<4>
    {
        using Type = std::codecvt_utf8<wchar_t>;
    };

    class UnicodeWstringConverter final : public WstringConverter
    {
    public:
        byte* toUTF8(const wchar_t* sourceStart, const wchar_t* sourceEnd, UTF8Buffer& buffer) const final
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
            if (sourceStart == sourceEnd)
            {
                return buffer.getMoreBytes(1, nullptr);
            }

            char* targetStart = nullptr;
            char* targetEnd = nullptr;
            char* targetNext = nullptr;

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

                targetStart =
                    reinterpret_cast<char*>(buffer.getMoreBytes(chunkSize, reinterpret_cast<byte*>(targetNext)));
                targetEnd = targetStart + chunkSize;
                targetNext = targetStart;

                codecvt_base::result result =
                    _codecvt.out(state, sourceStart, sourceEnd, sourceNext, targetStart, targetEnd, targetNext);

                switch (result)
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

                if (targetStart == targetNext)
                {
                    // We didn't convert a single character
                    throw IllegalConversionException(__FILE__, __LINE__, "no character converted by codecvt.out");
                }

                sourceStart = sourceNext;
            } while (more);

            return reinterpret_cast<byte*>(targetNext);
        }

        void fromUTF8(const byte* sourceStart, const byte* sourceEnd, wstring& target) const final
        {
            const auto sourceSize = static_cast<size_t>(sourceEnd - sourceStart);

            if (sourceSize == 0)
            {
                target = L"";
            }
            else
            {
                target.resize(sourceSize);
                auto* targetStart = const_cast<wchar_t*>(target.data());
                wchar_t* targetEnd = targetStart + sourceSize;
                wchar_t* targetNext = targetStart;

                const char* sourceNext = reinterpret_cast<const char*>(sourceStart);

                mbstate_t state = mbstate_t();

                codecvt_base::result result = _codecvt.in(
                    state,
                    reinterpret_cast<const char*>(sourceStart),
                    reinterpret_cast<const char*>(sourceEnd),
                    sourceNext,
                    targetStart,
                    targetEnd,
                    targetNext);

                if (result != codecvt_base::ok)
                {
                    throw IllegalConversionException(__FILE__, __LINE__, "codecvt.in failure");
                }

                target.resize(static_cast<size_t>(targetNext - targetStart));
            }
        }

    private:
        using CodeCvt = SelectCodeCvt<sizeof(wchar_t)>::Type;
        const CodeCvt _codecvt;
    };

    const WstringConverterPtr& getUnicodeWstringConverter()
    {
        static const WstringConverterPtr unicodeWstringConverter = make_shared<UnicodeWstringConverter>();
        return unicodeWstringConverter;
    }

    class UTF8BufferI final : public UTF8Buffer
    {
    public:
        //
        // Returns the first unused byte in the resized buffer
        //
        byte* getMoreBytes(size_t howMany, byte* firstUnused) final
        {
            size_t bytesUsed = 0;
            if (firstUnused)
            {
                bytesUsed = static_cast<size_t>(firstUnused - reinterpret_cast<const byte*>(_buffer.data()));
            }

            if (_buffer.size() < howMany + bytesUsed)
            {
                _buffer.resize(bytesUsed + howMany);
            }

            return const_cast<byte*>(reinterpret_cast<const byte*>(_buffer.data())) + bytesUsed;
        }

        void swap(string& other, const byte* tail)
        {
            assert(tail >= reinterpret_cast<const byte*>(_buffer.data()));
            _buffer.resize(static_cast<size_t>(tail - reinterpret_cast<const byte*>(_buffer.data())));
            other.swap(_buffer);
        }

    private:
        string _buffer;
    };
}

Ice::UTF8Buffer::~UTF8Buffer() = default; // Out of line to avoid weak vtable

WstringConverterPtr
Ice::createUnicodeWstringConverter()
{
    return getUnicodeWstringConverter();
}

StringConverterPtr
Ice::getProcessStringConverter()
{
    lock_guard lock(processStringConverterMutex);
    return processStringConverter;
}

void
Ice::setProcessStringConverter(const StringConverterPtr& converter)
{
    lock_guard lock(processStringConverterMutex);
    processStringConverter = converter;
}

WstringConverterPtr
Ice::getProcessWstringConverter()
{
    lock_guard lock(processStringConverterMutex);
    if (processWstringConverter)
    {
        return processWstringConverter;
    }
    else
    {
        return getUnicodeWstringConverter();
    }
}

void
Ice::setProcessWstringConverter(const WstringConverterPtr& converter)
{
    lock_guard lock(processStringConverterMutex);
    processWstringConverter = converter;
}

string
Ice::wstringToString(const wstring& v, const StringConverterPtr& converter, const WstringConverterPtr& wConverter)
{
    string target;
    if (!v.empty())
    {
        const WstringConverterPtr& wConverterWithDefault = wConverter ? wConverter : getUnicodeWstringConverter();

        //
        // First convert to UTF-8 narrow string.
        //
        UTF8BufferI buffer;
        byte* last = wConverterWithDefault->toUTF8(v.data(), v.data() + v.size(), buffer);
        buffer.swap(target, last);

        //
        // If narrow string converter is present convert to the native narrow string encoding, otherwise
        // native narrow string encoding is UTF8 and we are done.
        //
        if (converter)
        {
            string tmp;
            converter->fromUTF8(
                reinterpret_cast<const byte*>(target.data()),
                reinterpret_cast<const byte*>(target.data() + target.size()),
                tmp);
            tmp.swap(target);
        }
    }
    return target;
}

wstring
Ice::stringToWstring(const string& v, const StringConverterPtr& converter, const WstringConverterPtr& wConverter)
{
    wstring target;
    if (!v.empty())
    {
        //
        // If there is a narrow string converter use it to convert to UTF8, otherwise the narrow
        // string is already UTF8 encoded.
        //
        string tmp;
        if (converter)
        {
            UTF8BufferI buffer;
            byte* last = converter->toUTF8(v.data(), v.data() + v.size(), buffer);
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
        wConverterWithDefault->fromUTF8(
            reinterpret_cast<const byte*>(tmp.data()),
            reinterpret_cast<const byte*>(tmp.data() + tmp.size()),
            target);
    }
    return target;
}

string
Ice::nativeToUTF8(string_view str, const Ice::StringConverterPtr& converter)
{
    if (!converter || str.empty())
    {
        return string{str};
    }
    UTF8BufferI buffer;
    byte* last = converter->toUTF8(str.data(), str.data() + str.size(), buffer);
    string result;
    buffer.swap(result, last);
    return result;
}

string
Ice::UTF8ToNative(string_view str, const Ice::StringConverterPtr& converter)
{
    if (!converter || str.empty())
    {
        return string{str};
    }
    string tmp;
    converter->fromUTF8(
        reinterpret_cast<const byte*>(str.data()),
        reinterpret_cast<const byte*>(str.data() + str.size()),
        tmp);
    return tmp;
}

using Char16T = char16_t;
using Char32T = char32_t;

vector<unsigned short>
IceInternal::toUTF16(const vector<uint8_t>& source)
{
    vector<unsigned short> result;
    if (!source.empty())
    {
        static_assert(sizeof(Char16T) == sizeof(unsigned short));

        using Convert = wstring_convert<codecvt_utf8_utf16<Char16T>, Char16T>;

        Convert convert;

        try
        {
            Convert::wide_string ws = convert.from_bytes(
                reinterpret_cast<const char*>(&source.front()),
                reinterpret_cast<const char*>(&source.front() + source.size()));

            result = vector<unsigned short>(
                reinterpret_cast<const unsigned short*>(ws.data()),
                reinterpret_cast<const unsigned short*>(ws.data()) + ws.length());
        }
        catch (const std::range_error& ex)
        {
            throw Ice::IllegalConversionException(__FILE__, __LINE__, ex.what());
        }
    }
    return result;
}

vector<unsigned int>
IceInternal::toUTF32(const vector<uint8_t>& source)
{
    vector<unsigned int> result;
    if (!source.empty())
    {
        static_assert(sizeof(Char32T) == sizeof(unsigned int));

        using Convert = wstring_convert<codecvt_utf8<Char32T>, Char32T>;
        Convert convert;

        try
        {
            Convert::wide_string ws = convert.from_bytes(
                reinterpret_cast<const char*>(&source.front()),
                reinterpret_cast<const char*>(&source.front() + source.size()));

            result = vector<unsigned int>(
                reinterpret_cast<const unsigned int*>(ws.data()),
                reinterpret_cast<const unsigned int*>(ws.data()) + ws.length());
        }
        catch (const std::range_error& ex)
        {
            throw Ice::IllegalConversionException(__FILE__, __LINE__, ex.what());
        }
    }
    return result;
}

vector<uint8_t>
IceInternal::fromUTF32(const vector<unsigned int>& source)
{
    vector<uint8_t> result;
    if (!source.empty())
    {
        static_assert(sizeof(Char32T) == sizeof(unsigned int));

        using Convert = wstring_convert<codecvt_utf8<Char32T>, Char32T>;
        Convert convert;

        try
        {
            Convert::byte_string bs = convert.to_bytes(
                reinterpret_cast<const Char32T*>(&source.front()),
                reinterpret_cast<const Char32T*>(&source.front() + source.size()));

            result = vector<uint8_t>(
                reinterpret_cast<const uint8_t*>(bs.data()),
                reinterpret_cast<const uint8_t*>(bs.data()) + bs.length());
        }
        catch (const std::range_error& ex)
        {
            throw Ice::IllegalConversionException(__FILE__, __LINE__, ex.what());
        }
    }
    return result;
}

#ifdef _WIN32

namespace
{
    //
    // Converts to/from UTF-8 using MultiByteToWideChar and WideCharToMultiByte
    //
    class WindowsStringConverter final : public StringConverter
    {
    public:
        explicit WindowsStringConverter(unsigned int);

        byte* toUTF8(const char*, const char*, UTF8Buffer&) const final;

        void fromUTF8(const byte*, const byte*, string& target) const final;

    private:
        unsigned int _cp;
    };

    WindowsStringConverter::WindowsStringConverter(unsigned int cp) : _cp(cp) {}

    byte* WindowsStringConverter::toUTF8(const char* sourceStart, const char* sourceEnd, UTF8Buffer& buffer) const
    {
        //
        // First convert to UTF-16
        //
        int sourceSize = static_cast<int>(sourceEnd - sourceStart);
        if (sourceSize == 0)
        {
            return buffer.getMoreBytes(1, nullptr);
        }

        int writtenWchar = 0;
        wstring wbuffer;

        //
        // The following code pages doesn't support MB_ERR_INVALID_CHARS flag
        // see http://msdn.microsoft.com/en-us/library/windows/desktop/dd319072(v=vs.85).aspx
        //
        DWORD flags = (_cp == 50220 || _cp == 50221 || _cp == 50222 || _cp == 50225 || _cp == 50227 || _cp == 50229 ||
                       _cp == 65000 || _cp == 42 || (_cp >= 57002 && _cp <= 57011))
                          ? 0
                          : MB_ERR_INVALID_CHARS;

        do
        {
            wbuffer.resize(wbuffer.size() == 0 ? sourceSize + 2 : 2 * wbuffer.size());
            writtenWchar = MultiByteToWideChar(
                _cp,
                flags,
                sourceStart,
                sourceSize,
                const_cast<wchar_t*>(wbuffer.data()),
                static_cast<int>(wbuffer.size()));
        } while (writtenWchar == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

        if (writtenWchar == 0)
        {
            throw IllegalConversionException(__FILE__, __LINE__, IceInternal::lastErrorToString());
        }

        wbuffer.resize(static_cast<size_t>(writtenWchar));

        //
        // Then convert this UTF-16 wbuffer into UTF-8
        //
        return getUnicodeWstringConverter()->toUTF8(wbuffer.data(), wbuffer.data() + wbuffer.size(), buffer);
    }

    void WindowsStringConverter::fromUTF8(const byte* sourceStart, const byte* sourceEnd, string& target) const
    {
        if (sourceStart == sourceEnd)
        {
            target = "";
            return;
        }

        if (_cp == CP_UTF8)
        {
            string tmp(reinterpret_cast<const char*>(sourceStart), static_cast<size_t>(sourceEnd - sourceStart));
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
            target.resize(
                writtenChar == -1 ? std::max<size_t>(sourceEnd - sourceStart + 2, target.size()) : 2 * target.size());

            writtenChar = WideCharToMultiByte(
                _cp,
                flags,
                wtarget.data(),
                static_cast<int>(wtarget.size()),
                const_cast<char*>(target.data()),
                static_cast<int>(target.size()),
                0,
                0);
        } while (writtenChar == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

        if (writtenChar == 0)
        {
            throw IllegalConversionException(__FILE__, __LINE__, IceInternal::lastErrorToString());
        }

        target.resize(static_cast<size_t>(writtenChar));
    }
}

StringConverterPtr
Ice::createWindowsStringConverter(unsigned int cp)
{
    return make_shared<WindowsStringConverter>(cp);
}
#endif

const char*
Ice::IllegalConversionException::ice_id() const noexcept
{
    return "::Ice::IllegalConversionException";
}
