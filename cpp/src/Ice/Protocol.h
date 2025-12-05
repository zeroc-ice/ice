// Copyright (c) ZeroC, Inc.

#ifndef ICE_PROTOCOL_H
#define ICE_PROTOCOL_H

#include "Ice/Config.h"
#include "Ice/LocalExceptions.h"
#include "Ice/VersionFunctions.h"

namespace IceInternal
{
    /// Identifies the latest protocol version.
    constexpr Ice::ProtocolVersion currentProtocol{Ice::Protocol_1_0};

    /// Identifies the latest protocol encoding version.
    constexpr Ice::EncodingVersion currentProtocolEncoding{Ice::Encoding_1_0};

    //
    // Size of the Ice protocol header
    //
    // Magic number (4 Bytes)
    // Protocol version major (std::uint8_t)
    // Protocol version minor (std::uint8_t)
    // Encoding version major (std::uint8_t)
    // Encoding version minor (std::uint8_t)
    // Message type (std::uint8_t)
    // Compression status (std::uint8_t)
    // Message size (std::int32_t)
    //
    const std::int32_t headerSize = 14;

    //
    // The magic number at the front of each message
    //
    extern const std::byte magic[4];

    //
    // The Ice protocol message types
    //
    const std::uint8_t requestMsg = 0;
    const std::uint8_t requestBatchMsg = 1;
    const std::uint8_t replyMsg = 2;
    const std::uint8_t validateConnectionMsg = 3;
    const std::uint8_t closeConnectionMsg = 4;

    //
    // The request header, batch request header and reply header.
    //
    extern const std::byte requestHdr[headerSize + sizeof(std::int32_t)];
    extern const std::byte requestBatchHdr[headerSize + sizeof(std::int32_t)];
    extern const std::byte replyHdr[headerSize];

    //
    // IPv4/IPv6 support enumeration.
    //
    enum ProtocolSupport
    {
        EnableIPv4,
        EnableIPv6,
        EnableBoth
    };

    template<typename T> bool isSupported(const T& version, const T& supported)
    {
        return version.major == supported.major && version.minor <= supported.minor;
    }

    const std::uint8_t OPTIONAL_END_MARKER = 0xFF;

    const std::uint8_t FLAG_HAS_TYPE_ID_STRING = (1 << 0);
    const std::uint8_t FLAG_HAS_TYPE_ID_INDEX = (1 << 1);
    const std::uint8_t FLAG_HAS_TYPE_ID_COMPACT = (1 << 0) | (1 << 1);
    const std::uint8_t FLAG_HAS_OPTIONAL_MEMBERS = (1 << 2);
    const std::uint8_t FLAG_HAS_INDIRECTION_TABLE = (1 << 3);
    const std::uint8_t FLAG_HAS_SLICE_SIZE = (1 << 4);
    const std::uint8_t FLAG_IS_LAST_SLICE = (1 << 5);

    inline void checkSupportedEncoding(const Ice::EncodingVersion& v)
    {
        if (!isSupported(v, Ice::Encoding_1_1))
        {
            throw Ice::MarshalException{
                __FILE__,
                __LINE__,
                "this Ice runtime does not support encoding version " + Ice::encodingVersionToString(v)};
        }
    }
}

#endif
