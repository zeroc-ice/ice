// Copyright (c) ZeroC, Inc.

#ifndef ICE_STRING_CONVERTER_H
#define ICE_STRING_CONVERTER_H

#include "Config.h"
#include "LocalException.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Ice
{
    /// Provides bytes to toUTF8.
    /// @headerfile Ice/Ice.h
    class ICE_API UTF8Buffer
    {
    public:
        /// Obtains more bytes.
        /// @param howMany The number of bytes requested.
        /// @param firstUnused A pointer to the first unused byte.
        /// @return A pointer to the beginning of the buffer.
        /// @throws std::bad_alloc Thrown when too many bytes are requested.
        /// @throws MarshalException Thrown when too many bytes are requested.
        virtual std::byte* getMoreBytes(size_t howMany, std::byte* firstUnused) = 0;

        virtual ~UTF8Buffer();
    };

    /// A StringConverter converts narrow or wide-strings to and from UTF-8 byte sequences.
    /// It's used by the communicator during marshaling (toUTF8) and unmarshaling (fromUTF8).
    /// It report errors by throwing IllegalConversionException or an exception thrown
    /// by UTF8Buffer.
    /// @headerfile Ice/Ice.h
    template<typename charT> class BasicStringConverter
    {
    public:
        /// Returns a pointer to byte after the last written byte (which may be
        /// past the last byte returned by getMoreBytes).
        virtual std::byte* toUTF8(const charT* sourceStart, const charT* sourceEnd, UTF8Buffer& buf) const = 0;

        /// Unmarshals a UTF-8 sequence into a basic_string.
        virtual void
        fromUTF8(const std::byte* sourceStart, const std::byte* sourceEnd, std::basic_string<charT>& target) const = 0;

        virtual ~BasicStringConverter() = default;
    };

#if defined(__clang__)
    // Explicit template instantiation so that dynamic_cast of derived exported
    // classes works well with clang, see ICE-7473.
    template class __attribute__((visibility("default"))) BasicStringConverter<char>;
    template class __attribute__((visibility("default"))) BasicStringConverter<wchar_t>;
#endif

    /// A narrow string converter.
    using StringConverter = BasicStringConverter<char>;

    /// A shared pointer to a StringConverter.
    using StringConverterPtr = std::shared_ptr<StringConverter>;

    /// A wide string converter.
    using WstringConverter = BasicStringConverter<wchar_t>;

    /// A shared pointer to a WstringConverter.
    using WstringConverterPtr = std::shared_ptr<WstringConverter>;

    /// Creates a WstringConverter that converts to and from UTF-16 or UTF-32 depending on sizeof(wchar_t).
    /// @return The string converter.
    ICE_API WstringConverterPtr createUnicodeWstringConverter();

    /// Retrieves the per-process narrow string converter.
    /// @return The string converter.
    ICE_API StringConverterPtr getProcessStringConverter();

    /// Sets the per-process narrow string converter.
    /// @param c The string converter.
    ICE_API void setProcessStringConverter(const StringConverterPtr& c);

    /// Retrieves the per-process wide string converter.
    /// @return The string converter.
    ICE_API WstringConverterPtr getProcessWstringConverter();

    /// Sets the per process wide string converter.
    /// @param c The string converter.
    ICE_API void setProcessWstringConverter(const WstringConverterPtr& c);

    /// Converts the given wide string to a narrow string.
    /// @param str A wide string.
    /// @param nc The narrow string converter. If null, the result's narrow string encoding is UTF-8.
    /// @param wc The wide string converter. If null, the input's wstring encoding is UTF-16 or UTF-32
    /// depending on the size of wchar_t.
    /// @return A narrow string.
    ICE_API std::string wstringToString(
        const std::wstring& str,
        const StringConverterPtr& nc = nullptr,
        const WstringConverterPtr& wc = nullptr);

    /// Converts the given narrow string to a wide string.
    /// @param str A narrow string.
    /// @param nc The narrow string converter. If null, the result's narrow string encoding is UTF-8.
    /// @param wc The wide string converter. If null, the input's wstring encoding is UTF-16 or UTF-32
    /// depending on the size of wchar_t.
    /// @return A wide string.
    ICE_API std::wstring stringToWstring(
        const std::string& str,
        const StringConverterPtr& nc = nullptr,
        const WstringConverterPtr& wc = nullptr);

    /// Converts the given string from the native narrow string encoding to
    /// UTF-8 using the given converter.
    /// @param str The native narrow string.
    /// @param nc The narrow string converter. If null, the native string is returned.
    /// @return A UTF-8 string.
    ICE_API std::string nativeToUTF8(std::string_view str, const StringConverterPtr& nc);

    /// Converts the given string from UTF-8 to the native narrow string
    /// encoding using the given converter.
    /// @param str The UTF-8 string.
    /// @param nc The narrow string converter. If null, the UTF-8 string is returned.
    /// @return A native narrow string.
    ICE_API std::string UTF8ToNative(std::string_view str, const StringConverterPtr& nc);

#if defined(_WIN32) || defined(ICE_DOXYGEN)
    /// Creates a StringConverter that converts to and from narrow chars
    /// in the given code page, using MultiByteToWideChar and WideCharToMultiByte.
    /// @param page The code page.
    /// @return The string converter.
    /// @remark Windows only.
    ICE_API StringConverterPtr createWindowsStringConverter(unsigned int page);
#endif

    /// This exception indicates the failure of a string conversion.
    /// @headerfile Ice/Ice.h
    class ICE_API IllegalConversionException final : public LocalException
    {
    public:
        /// Constructs an IllegalConversionException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param message The error message adopted by this exception and returned by what().
        IllegalConversionException(const char* file, int line, std::string message)
            : LocalException(file, line, std::move(message))
        {
        }

        [[nodiscard]] const char* ice_id() const noexcept final;
    };
}

namespace IceInternal
{
    //
    // Convert from UTF-8 to UTF-16/32
    //
    ICE_API std::vector<unsigned short> toUTF16(const std::vector<std::uint8_t>&);
    ICE_API std::vector<unsigned int> toUTF32(const std::vector<std::uint8_t>&);

    //
    // Convert from UTF-32 to UTF-8
    //
    ICE_API std::vector<std::uint8_t> fromUTF32(const std::vector<unsigned int>&);
}

#endif
