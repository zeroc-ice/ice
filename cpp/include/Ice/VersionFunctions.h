//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_VERSION_FUNCTIONS_H
#define ICE_VERSION_FUNCTIONS_H

#include "Config.h"
#include "Ice/Version.h"

#include <ostream>

namespace Ice
{
    /** Identifies protocol version 1.0. */
    constexpr ProtocolVersion Protocol_1_0 = {1, 0};

    /** Identifies encoding version 1.0. */
    constexpr EncodingVersion Encoding_1_0 = {1, 0};

    /** Identifies encoding version 1.1. */
    constexpr EncodingVersion Encoding_1_1 = {1, 1};

    /** Identifies the latest protocol version. */
    constexpr ProtocolVersion currentProtocol = Protocol_1_0;

    /** Identifies the latest protocol encoding version. */
    constexpr EncodingVersion currentProtocolEncoding = Encoding_1_0;

    /** Identifies the latest encoding version. */
    constexpr EncodingVersion currentEncoding = Encoding_1_1;

    /**
     * Converts a protocol version into a string.
     * @param v The protocol version.
     * @return A string representing the protocol version.
     */
    ICE_API std::string protocolVersionToString(const ProtocolVersion& v);

    /**
     * Converts a string into a protocol version.
     * @param v The string containing a stringified protocol version.
     * @return The protocol version.
     * @throws VersionParseException If the given string is not in the X.Y format.
     */
    ICE_API ProtocolVersion stringToProtocolVersion(std::string_view v);

    /**
     * Converts an encoding version into a string.
     * @param v The encoding version.
     * @return A string representing the encoding version.
     */
    ICE_API std::string encodingVersionToString(const EncodingVersion& v);

    /**
     * Converts a string into an encoding version.
     * @param v The string containing a stringified encoding version.
     * @return The encoding version.
     * @throws VersionParseException If the given string is not in the X.Y format.
     */
    ICE_API EncodingVersion stringToEncodingVersion(std::string_view v);

    ICE_API std::ostream& operator<<(std::ostream& out, const ProtocolVersion& version);

    ICE_API std::ostream& operator<<(std::ostream& out, const EncodingVersion& version);
}

#endif
