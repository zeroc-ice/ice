//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PROTOCOL_H
#define ICE_PROTOCOL_H

#include "Ice/Config.h"
#include "Ice/VersionFunctions.h"

#include <sstream>

namespace IceInternal
{
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
    // Message size (Int)
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

    template<typename T> bool isSupported(const T& version, const T& supported)
    {
        return version.major == supported.major && version.minor <= supported.minor;
    }

    ICE_API void
    throwUnsupportedProtocolException(const char*, int, const Ice::ProtocolVersion&, const Ice::ProtocolVersion&);
    ICE_API void
    throwUnsupportedEncodingException(const char*, int, const Ice::EncodingVersion&, const Ice::EncodingVersion&);

    const std::uint8_t OPTIONAL_END_MARKER = 0xFF;

    const std::uint8_t FLAG_HAS_TYPE_ID_STRING = (1 << 0);
    const std::uint8_t FLAG_HAS_TYPE_ID_INDEX = (1 << 1);
    const std::uint8_t FLAG_HAS_TYPE_ID_COMPACT = (1 << 0) | (1 << 1);
    const std::uint8_t FLAG_HAS_OPTIONAL_MEMBERS = (1 << 2);
    const std::uint8_t FLAG_HAS_INDIRECTION_TABLE = (1 << 3);
    const std::uint8_t FLAG_HAS_SLICE_SIZE = (1 << 4);
    const std::uint8_t FLAG_IS_LAST_SLICE = (1 << 5);

    inline void checkSupportedProtocol(const Ice::ProtocolVersion& v)
    {
        if (!isSupported(v, Ice::currentProtocol))
        {
            throwUnsupportedProtocolException(__FILE__, __LINE__, v, Ice::currentProtocol);
        }
    }

    inline void checkSupportedProtocolEncoding(const Ice::EncodingVersion& v)
    {
        if (!isSupported(v, Ice::currentProtocolEncoding))
        {
            throwUnsupportedEncodingException(__FILE__, __LINE__, v, Ice::currentProtocolEncoding);
        }
    }

    inline void checkSupportedEncoding(const Ice::EncodingVersion& v)
    {
        if (!isSupported(v, Ice::currentEncoding))
        {
            throwUnsupportedEncodingException(__FILE__, __LINE__, v, Ice::currentEncoding);
        }
    }
}

#endif
