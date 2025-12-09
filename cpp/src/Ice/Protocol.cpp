// Copyright (c) ZeroC, Inc.

#include "Protocol.h"
#include "Ice/LocalExceptions.h"

#include <sstream>

using namespace std;
using namespace Ice;

namespace
{
    template<typename T> std::string versionToString(const T& v)
    {
        std::ostringstream os;
        os << v;
        return os.str();
    }

    template<typename T> T stringToVersion(std::string_view str)
    {
        T v;
        IceInternal::stringToMajorMinor(str, v.major, v.minor);
        return v;
    }
}

string
IceInternal::protocolVersionToString(const ProtocolVersion& v)
{
    return versionToString<ProtocolVersion>(v);
}

ProtocolVersion
IceInternal::stringToProtocolVersion(string_view v)
{
    return stringToVersion<ProtocolVersion>(v);
}

string
IceInternal::encodingVersionToString(const EncodingVersion& v)
{
    return versionToString<EncodingVersion>(v);
}

EncodingVersion
IceInternal::stringToEncodingVersion(string_view v)
{
    return stringToVersion<EncodingVersion>(v);
}

ostream&
Ice::operator<<(ostream& out, const ProtocolVersion& version)
{
    return out << static_cast<int>(version.major) << "." << static_cast<int>(version.minor);
}

ostream&
Ice::operator<<(ostream& out, const EncodingVersion& version)
{
    return out << static_cast<int>(version.major) << "." << static_cast<int>(version.minor);
}

void
IceInternal::stringToMajorMinor(string_view s, uint8_t& major, uint8_t& minor)
{
    string str{s};
    std::string::size_type pos = str.find_first_of('.');
    if (pos == std::string::npos)
    {
        throw ParseException(__FILE__, __LINE__, "malformed Ice version value '" + str + "'");
    }

    std::istringstream majStr(str.substr(0, pos));
    int32_t majVersion;
    if (!(majStr >> majVersion) || !majStr.eof())
    {
        throw ParseException(__FILE__, __LINE__, "invalid major Ice version value '" + str + "'");
    }

    std::istringstream minStr(str.substr(pos + 1, std::string::npos));
    int32_t minVersion;
    if (!(minStr >> minVersion) || !minStr.eof())
    {
        throw ParseException(__FILE__, __LINE__, "invalid minor Ice version value '" + str + "'");
    }

    if (majVersion < 1 || majVersion > 255 || minVersion < 0 || minVersion > 255)
    {
        throw ParseException(__FILE__, __LINE__, "range error in version '" + str + "'");
    }

    major = static_cast<uint8_t>(majVersion);
    minor = static_cast<uint8_t>(minVersion);
}

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
}
