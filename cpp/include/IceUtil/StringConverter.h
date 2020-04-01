//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_STRING_CONVERTER_H
#define ICE_UTIL_STRING_CONVERTER_H

#include <IceUtil/Config.h>
#include <IceUtil/Exception.h>
#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>

#include <string>

namespace IceUtil
{

/**
 * Provides bytes to toUTF8. Can raise std::bad_alloc or Ice::MemoryLimitException
 * when too many bytes are requested.
 * \headerfile Ice/Ice.h
 */
class ICE_API UTF8Buffer
{
public:

    /**
     * Obtains more bytes.
     * @param howMany The number of bytes requested.
     * @param firstUnused A pointer to the first unused byte.
     * @return A pointer to the beginning of the buffer.
     */
    virtual Byte* getMoreBytes(size_t howMany, Byte* firstUnused) = 0;

    virtual ~UTF8Buffer();
};

/**
 * A StringConverter converts narrow or wide-strings to and from UTF-8 byte sequences.
 * It's used by the communicator during marshaling (toUTF8) and unmarshaling (fromUTF8).
 * It report errors by raising IllegalConversionException or an exception raised
 * by UTF8Buffer.
 * \headerfile Ice/Ice.h
 */
template<typename charT>
class BasicStringConverter
{
public:

    /**
     * Returns a pointer to byte after the last written byte (which may be
     * past the last byte returned by getMoreBytes).
     */
    virtual Byte* toUTF8(const charT* sourceStart, const charT* sourceEnd, UTF8Buffer& buf) const = 0;

    /**
     * Unmarshals a UTF-8 sequence into a basic_string.
     */
    virtual void fromUTF8(const Byte* sourceStart, const Byte* sourceEnd, std::basic_string<charT>& target) const = 0;

    virtual ~BasicStringConverter()
    {
    }
};

#if defined(__clang__) && !defined(ICE_STATIC_LIBS)
//
// Explicit template instantiation so that dynamic_cast of derived exported
// classes works well with clang, see ICE-7473.
//
template class ICE_API BasicStringConverter<char>;
template class ICE_API BasicStringConverter<wchar_t>;
#endif

/** A narrow string converter. */
typedef BasicStringConverter<char> StringConverter;
ICE_DEFINE_PTR(StringConverterPtr, StringConverter);

/** A wide string converter. */
typedef BasicStringConverter<wchar_t> WstringConverter;
ICE_DEFINE_PTR(WstringConverterPtr, WstringConverter);

/**
 * Creates a WstringConverter that converts to and from UTF-16 or UTF-32 depending on sizeof(wchar_t).
 * @return The string converter.
 */
ICE_API WstringConverterPtr createUnicodeWstringConverter();

/**
 * Retrieves the per-process narrow string converter.
 * @return The string converter.
 */
ICE_API StringConverterPtr getProcessStringConverter();

/**
 * Sets the per-process narrow string converter.
 * @param c The string converter.
 */
ICE_API void setProcessStringConverter(const StringConverterPtr& c);

/**
 * Retrieves the per-process wide string converter.
 * @return The string converter.
 */
ICE_API WstringConverterPtr getProcessWstringConverter();

/**
 * Sets the per process wide string converter.
 * @param c The string converter.
 */
ICE_API void setProcessWstringConverter(const WstringConverterPtr& c);

/**
 * Converts the given wide string to a narrow string.
 * @param str A wide string.
 * @param nc The narrow string converter. If null, the result's narrow string encoding is UTF-8.
 * @param wc The wide string converter. If null, the input's wstring encoding is UTF-16 or UTF-32
 * depending on the size of wchar_t.
 * @return A narrow string.
 */
ICE_API std::string
wstringToString(const std::wstring& str,
                const StringConverterPtr& nc = 0,
                const WstringConverterPtr& wc = 0);

/**
 * Converts the given narrow string to a wide string.
 * @param str A narrow string.
 * @param nc The narrow string converter. If null, the result's narrow string encoding is UTF-8.
 * @param wc The wide string converter. If null, the input's wstring encoding is UTF-16 or UTF-32
 * depending on the size of wchar_t.
 * @return A wide string.
 */
ICE_API std::wstring
stringToWstring(const std::string& str,
                const StringConverterPtr& nc = 0,
                const WstringConverterPtr& wc = 0);

/**
 * Converts the given string from the native narrow string encoding to
 * UTF-8 using the given converter.
 * @param str The native narrow string.
 * @param nc The narrow string converter. If null, the native string is returned.
 * @return A UTF-8 string.
 */
ICE_API std::string
nativeToUTF8(const std::string& str, const StringConverterPtr& nc);

/**
 * Converts the given string from UTF-8 to the native narrow string
 * encoding using the given converter.
 * @param str The UTF-8 string.
 * @param nc The narrow string converter. If null, the UTF-8 string is returned.
 * @return A native narrow string.
 */
ICE_API std::string
UTF8ToNative(const std::string& str, const StringConverterPtr& nc);

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

/**
 * Creates a StringConverter that converts to and from narrow chars
 * in the given code page, using MultiByteToWideChar and WideCharToMultiByte.
 * @param page The code page.
 * @return The string converter.
 */
ICE_API StringConverterPtr createWindowsStringConverter(unsigned int page);

}
#endif

#endif
