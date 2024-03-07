//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Protocol.h>
#include <Ice/LocalException.h>

using namespace std;

namespace IceInternal
{
    const byte magic[] = {byte{0x49}, byte{0x63}, byte{0x65}, byte{0x50}}; // 'I', 'c', 'e', 'P'

    const byte requestHdr[] = {
        magic[0],
        magic[1],
        magic[2],
        magic[3],
        byte{protocolMajor},
        byte{protocolMinor},
        byte{protocolEncodingMajor},
        byte{protocolEncodingMinor},
        byte{requestMsg},
        byte{0}, // Compression status
        byte{0},
        byte{0},
        byte{0},
        byte{0}, // Message size (placeholder)
        byte{0},
        byte{0},
        byte{0},
        byte{0} // Request id (placeholder)
    };

    const byte requestBatchHdr[] = {
        magic[0],
        magic[1],
        magic[2],
        magic[3],
        byte{protocolMajor},
        byte{protocolMinor},
        byte{protocolEncodingMajor},
        byte{protocolEncodingMinor},
        byte{requestBatchMsg},
        byte{0}, // Compression status
        byte{0},
        byte{0},
        byte{0},
        byte{0}, // Message size (place holder)
        byte{0},
        byte{0},
        byte{0},
        byte{0} // Number of requests in batch (placeholder)
    };

    const byte replyHdr[] = {
        magic[0],
        magic[1],
        magic[2],
        magic[3],
        byte{protocolMajor},
        byte{protocolMinor},
        byte{protocolEncodingMajor},
        byte{protocolEncodingMinor},
        byte{replyMsg},
        byte{0}, // Compression status
        byte{0},
        byte{0},
        byte{0},
        byte{0} // Message size (placeholder)
    };

    void stringToMajorMinor(const std::string& str, uint8_t& major, uint8_t& minor)
    {
        std::string::size_type pos = str.find_first_of(".");
        if (pos == std::string::npos)
        {
            throw Ice::VersionParseException(__FILE__, __LINE__, "malformed version value `" + str + "'");
        }

        std::istringstream majStr(str.substr(0, pos));
        int32_t majVersion;
        if (!(majStr >> majVersion) || !majStr.eof())
        {
            throw Ice::VersionParseException(__FILE__, __LINE__, "invalid major version value `" + str + "'");
        }

        std::istringstream minStr(str.substr(pos + 1, std::string::npos));
        int32_t minVersion;
        if (!(minStr >> minVersion) || !minStr.eof())
        {
            throw Ice::VersionParseException(__FILE__, __LINE__, "invalid minor version value `" + str + "'");
        }

        if (majVersion < 1 || majVersion > 255 || minVersion < 0 || minVersion > 255)
        {
            throw Ice::VersionParseException(__FILE__, __LINE__, "range error in version `" + str + "'");
        }

        major = static_cast<uint8_t>(majVersion);
        minor = static_cast<uint8_t>(minVersion);
    }

    void throwUnsupportedProtocolException(
        const char* f,
        int l,
        const Ice::ProtocolVersion& v,
        const Ice::ProtocolVersion& s)
    {
        throw Ice::UnsupportedProtocolException(f, l, "", v, s);
    }

    void throwUnsupportedEncodingException(
        const char* f,
        int l,
        const Ice::EncodingVersion& v,
        const Ice::EncodingVersion& s)
    {
        throw Ice::UnsupportedEncodingException(f, l, "", v, s);
    }
}

namespace Ice
{
    const EncodingVersion currentEncoding = {IceInternal::encodingMajor, IceInternal::encodingMinor};
    const ProtocolVersion currentProtocol = {IceInternal::protocolMajor, IceInternal::protocolMinor};
    //
    // The encoding to use for protocol messages, this version is tied to
    // the protocol version.
    //

    const EncodingVersion currentProtocolEncoding = {
        IceInternal::protocolEncodingMajor, IceInternal::protocolEncodingMinor};

    const ProtocolVersion Protocol_1_0 = {1, 0};

    const EncodingVersion Encoding_1_0 = {1, 0};
    const EncodingVersion Encoding_1_1 = {1, 1};
}
