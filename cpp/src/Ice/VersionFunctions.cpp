// Copyright (c) ZeroC, Inc.

#include "Ice/VersionFunctions.h"
#include "Ice/LocalExceptions.h"
#include "Protocol.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

string
Ice::protocolVersionToString(const ProtocolVersion& v)
{
    return versionToString<ProtocolVersion>(v);
}

ProtocolVersion
Ice::stringToProtocolVersion(string_view v)
{
    return stringToVersion<ProtocolVersion>(v);
}

string
Ice::encodingVersionToString(const EncodingVersion& v)
{
    return versionToString<EncodingVersion>(v);
}

EncodingVersion
Ice::stringToEncodingVersion(string_view v)
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
