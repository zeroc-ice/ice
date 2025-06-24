// Copyright (c) ZeroC, Inc.

#ifndef ICE_STRING_UTIL_H
#define ICE_STRING_UTIL_H

#include "Ice/Config.h"

#include <cstdint>
#include <string>
#include <vector>

namespace Ice
{
    /// The output mode for xxxToString methods such as identityToString and proxyToString. The actual encoding format
    /// for the string is the same for all modes: you don't need to specify an encoding format or mode when reading such
    /// a string.
    enum class ToStringMode : std::uint8_t
    {
        /// Characters with ordinal values greater than 127 are kept as-is in the resulting string. Non-printable ASCII
        /// characters with ordinal values 127 and below are encoded as \\t, \\n (etc.) or \\unnnn.
        Unicode,
        /// Characters with ordinal values greater than 127 are encoded as universal character names in the resulting
        /// string: \\unnnn for BMP characters and \\Unnnnnnnn for non-BMP characters. Non-printable ASCII characters
        /// with ordinal values 127 and below are encoded as \\t, \\n (etc.) or \\unnnn.
        ASCII,
        /// Characters with ordinal values greater than 127 are encoded as a sequence of UTF-8 bytes using octal
        /// escapes. Characters with ordinal values 127 and below are encoded as \\t, \\n (etc.) or an octal escape. Use
        /// this mode to generate strings compatible with Ice 3.6 and earlier.
        Compat
    };
}

namespace IceInternal
{
    //
    // Add escape sequences (like "\n", or "\123") to the input string
    // (first parameter).
    // The second parameter adds characters to escape, and can be empty.
    //
    ICE_API std::string escapeString(std::string_view s, std::string_view special, Ice::ToStringMode toStringMode);

    //
    // Remove escape sequences added by escapeString. Throws std::invalid_argument
    // for an invalid input string.
    //
    ICE_API std::string unescapeString(
        std::string_view,
        std::string_view::size_type,
        std::string_view::size_type,
        std::string_view special);

    //
    // Split a string using the given delimiters. Considers single and double quotes;
    // returns false for unbalanced quote, true otherwise.
    //
    ICE_API bool splitString(std::string_view, std::string_view, std::vector<std::string>&);

    //
    // Join a list of strings using the given delimiter.
    //
    ICE_API std::string joinString(const std::vector<std::string>&, std::string_view delimiter);

    //
    // Trim white space
    //
    ICE_API std::string trim(std::string_view);

    //
    // If a single or double quotation mark is found at the start
    // position, then the position of the matching closing quote is
    // returned. If no quotation mark is found at the start position, then
    // 0 is returned. If no matching closing quote is found, then
    // std::string::npos is returned.
    //
    ICE_API std::string::size_type checkQuote(const std::string&, std::string::size_type = 0);

    //
    // Match `s' against the pattern `pat'. A * in the pattern acts
    // as a wildcard: it matches any non-empty sequence of characters
    // other than a period (`.'). We match by hand here because
    // it's portable across platforms (whereas regex() isn't).
    //
    ICE_API bool match(const std::string&, const std::string&, bool = false);

    //
    // Get the error message for the last error code or given error code.
    //
    ICE_API std::string lastErrorToString();
#ifdef _WIN32
    using ErrorCode = unsigned long; // DWORD
    ICE_API std::string errorToStringWithSource(ErrorCode error, const void* source);
    inline std::string errorToString(ErrorCode error) { return errorToStringWithSource(error, nullptr); }
#else
    using ErrorCode = int;
    ICE_API std::string errorToString(int);
#endif

    //
    // Functions to convert to lower/upper case. These functions accept
    // UTF8 string/characters but ignore non ASCII characters. Unlike, the
    // C methods, these methods are not local dependent.
    //
    ICE_API std::string toLower(std::string_view);
    ICE_API std::string toUpper(std::string_view);
    ICE_API bool isAlpha(char);
    ICE_API bool isDigit(char);

    //
    // Remove all whitespace from a string
    //
    ICE_API std::string removeWhitespace(std::string_view);
}

#endif
