//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Protocol.h"
#include "Ice/LocalException.h"

using namespace std;
using namespace Ice;

namespace IceInternal
{
    const byte magic[] = {byte{0x49}, byte{0x63}, byte{0x65}, byte{0x50}}; // 'I', 'c', 'e', 'P'

    const byte requestHdr[] = {
        magic[0],
        magic[1],
        magic[2],
        magic[3],
        byte{currentProtocol.major},
        byte{currentProtocol.minor},
        byte{currentProtocolEncoding.major},
        byte{currentProtocolEncoding.minor},
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
        byte{currentProtocol.major},
        byte{currentProtocol.minor},
        byte{currentProtocolEncoding.major},
        byte{currentProtocolEncoding.minor},
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
        byte{currentProtocol.major},
        byte{currentProtocol.minor},
        byte{currentProtocolEncoding.major},
        byte{currentProtocolEncoding.minor},
        byte{replyMsg},
        byte{0}, // Compression status
        byte{0},
        byte{0},
        byte{0},
        byte{0} // Message size (placeholder)
    };

    void stringToMajorMinor(string_view s, uint8_t& major, uint8_t& minor)
    {
        string str{s};
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
