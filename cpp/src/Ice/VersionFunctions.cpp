//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/VersionFunctions.h"
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
