// Copyright (c) ZeroC, Inc.

#ifndef ICE_VERSION_FUNCTIONS_H
#define ICE_VERSION_FUNCTIONS_H

#include "Config.h"
#include "Ice/Version.h"

#include <sstream>

namespace Ice
{
    /// Identifies protocol version 1.0.
    constexpr ProtocolVersion Protocol_1_0{1, 0};

    /// Identifies encoding version 1.0.
    constexpr EncodingVersion Encoding_1_0{1, 0};

    /// Identifies encoding version 1.1.
    constexpr EncodingVersion Encoding_1_1{1, 1};

    /// Converts a protocol version into a string.
    /// @param v The protocol version.
    /// @return A string representing the protocol version.
    ICE_API std::string protocolVersionToString(const ProtocolVersion& v);

    /// Converts a string into a protocol version.
    /// @param v The string containing a stringified protocol version.
    /// @return The protocol version.
    /// @throws ParseException If the given string is not in the X.Y format.
    ICE_API ProtocolVersion stringToProtocolVersion(std::string_view v);

    /// Converts an encoding version into a string.
    /// @param v The encoding version.
    /// @return A string representing the encoding version.
    ICE_API std::string encodingVersionToString(const EncodingVersion& v);

    /// Converts a string into an encoding version.
    /// @param v The string containing a stringified encoding version.
    /// @return The encoding version.
    /// @throws ParseException If the given string is not in the X.Y format.
    ICE_API EncodingVersion stringToEncodingVersion(std::string_view v);
}

namespace IceInternal
{
    ICE_API void stringToMajorMinor(std::string_view, std::uint8_t&, std::uint8_t&);

    template<typename T> std::string versionToString(const T& v)
    {
        std::ostringstream os;
        os << v;
        return os.str();
    }

    template<typename T> T stringToVersion(std::string_view str)
    {
        T v;
        stringToMajorMinor(str, v.major, v.minor);
        return v;
    }
}

#endif
