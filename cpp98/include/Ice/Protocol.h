//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PROTOCOL_H
#define ICE_PROTOCOL_H

#include <Ice/Config.h>
#include <Ice/Version.h>

namespace IceInternal
{

//
// Size of the Ice protocol header
//
// Magic number (4 Bytes)
// Protocol version major (Byte)
// Protocol version minor (Byte)
// Encoding version major (Byte)
// Encoding version minor (Byte)
// Message type (Byte)
// Compression status (Byte)
// Message size (Int)
//
const ::Ice::Int headerSize = 14;

//
// The magic number at the front of each message
//
extern const ::Ice::Byte magic[4];

//
// The current Ice protocol, protocol encoding and encoding version
//
const ::Ice::Byte protocolMajor = 1;
const ::Ice::Byte protocolMinor = 0;
const ::Ice::Byte protocolEncodingMajor = 1;
const ::Ice::Byte protocolEncodingMinor = 0;

const ::Ice::Byte encodingMajor = 1;
const ::Ice::Byte encodingMinor = 1;

//
// The Ice protocol message types
//
const ::Ice::Byte requestMsg = 0;
const ::Ice::Byte requestBatchMsg = 1;
const ::Ice::Byte replyMsg = 2;
const ::Ice::Byte validateConnectionMsg = 3;
const ::Ice::Byte closeConnectionMsg = 4;

//
// The request header, batch request header and reply header.
//
extern const ::Ice::Byte requestHdr[headerSize + sizeof(Ice::Int)];
extern const ::Ice::Byte requestBatchHdr[headerSize + sizeof(Ice::Int)];
extern const ::Ice::Byte replyHdr[headerSize];

//
// IPv4/IPv6 support enumeration.
//
enum ProtocolSupport
{
    EnableIPv4,
    EnableIPv6,
    EnableBoth
};

ICE_API void stringToMajorMinor(const ::std::string&, Ice::Byte&, Ice::Byte&);

template<typename T> std::string
versionToString(const T& v)
{
    std::ostringstream os;
    os << v;
    return os.str();
}

template<typename T> T
stringToVersion(const ::std::string& str)
{
    T v;
    stringToMajorMinor(str, v.major, v.minor);
    return v;
}

template<typename T> bool
isSupported(const T& version, const T& supported)
{
    return version.major == supported.major && version.minor <= supported.minor;
}

ICE_API void throwUnsupportedProtocolException(const char*, int, const Ice::ProtocolVersion&,
                                               const Ice::ProtocolVersion&);
ICE_API void throwUnsupportedEncodingException(const char*, int, const Ice::EncodingVersion&,
                                               const Ice::EncodingVersion&);

const ::Ice::Byte OPTIONAL_END_MARKER        = 0xFF;

const ::Ice::Byte FLAG_HAS_TYPE_ID_STRING    = (1<<0);
const ::Ice::Byte FLAG_HAS_TYPE_ID_INDEX     = (1<<1);
const ::Ice::Byte FLAG_HAS_TYPE_ID_COMPACT   = (1<<0) | (1<<1);
const ::Ice::Byte FLAG_HAS_OPTIONAL_MEMBERS  = (1<<2);
const ::Ice::Byte FLAG_HAS_INDIRECTION_TABLE = (1<<3);
const ::Ice::Byte FLAG_HAS_SLICE_SIZE        = (1<<4);
const ::Ice::Byte FLAG_IS_LAST_SLICE         = (1<<5);

}

namespace Ice
{

/** Identifies protocol version 1.0. */
ICE_API extern const ProtocolVersion Protocol_1_0;

/** Identifies encoding version 1.0. */
ICE_API extern const EncodingVersion Encoding_1_0;

/** Identifies encoding version 1.1. */
ICE_API extern const EncodingVersion Encoding_1_1;

/** Identifies the latest protocol version. */
ICE_API extern const ProtocolVersion currentProtocol;

/** Identifies the latest protocol encoding version. */
ICE_API extern const EncodingVersion currentProtocolEncoding;

/** Identifies the latest encoding version. */
ICE_API extern const EncodingVersion currentEncoding;

/**
 * Converts a protocol version into a string.
 * @param v The protocol version.
 * @return A string representing the protocol version.
 */
inline ::std::string
protocolVersionToString(const Ice::ProtocolVersion& v)
{
    return IceInternal::versionToString<ProtocolVersion>(v);
}

/**
 * Converts a string into a protocol version.
 * @param v The string containing a stringified protocol version.
 * @return The protocol version.
 * @throws VersionParseException If the given string is not in the X.Y format.
 */
inline ::Ice::ProtocolVersion
stringToProtocolVersion(const ::std::string& v)
{
    return IceInternal::stringToVersion<ProtocolVersion>(v);
}

/**
 * Converts an encoding version into a string.
 * @param v The encoding version.
 * @return A string representing the encoding version.
 */
inline ::std::string
encodingVersionToString(const Ice::EncodingVersion& v)
{
    return IceInternal::versionToString<EncodingVersion>(v);
}

/**
 * Converts a string into an encoding version.
 * @param v The string containing a stringified encoding version.
 * @return The encoding version.
 * @throws VersionParseException If the given string is not in the X.Y format.
 */
inline ::Ice::EncodingVersion
stringToEncodingVersion(const ::std::string& v)
{
    return IceInternal::stringToVersion<EncodingVersion>(v);
}

inline std::ostream&
operator<<(std::ostream& out, const ProtocolVersion& version)
{
    return out << static_cast<int>(version.major) << "." << static_cast<int>(version.minor);
}

inline std::ostream&
operator<<(std::ostream& out, const EncodingVersion& version)
{
    return out << static_cast<int>(version.major) << "." << static_cast<int>(version.minor);
}

}

namespace IceInternal
{

inline void
checkSupportedProtocol(const Ice::ProtocolVersion& v)
{
    if(!isSupported(v, Ice::currentProtocol))
    {
        throwUnsupportedProtocolException(__FILE__, __LINE__, v, Ice::currentProtocol);
    }
}

inline void
checkSupportedProtocolEncoding(const Ice::EncodingVersion& v)
{
    if(!isSupported(v, Ice::currentProtocolEncoding))
    {
        throwUnsupportedEncodingException(__FILE__, __LINE__, v, Ice::currentProtocolEncoding);
    }
}

inline void
checkSupportedEncoding(const Ice::EncodingVersion& v)
{
    if(!isSupported(v, Ice::currentEncoding))
    {
        throwUnsupportedEncodingException(__FILE__, __LINE__, v, Ice::currentEncoding);
    }
}

//
// Either return the given protocol if not compatible, or the greatest
// supported protocol otherwise.
//
inline const Ice::ProtocolVersion
getCompatibleProtocol(const Ice::ProtocolVersion& v)
{
    if(v.major != Ice::currentProtocol.major)
    {
        return v; // Unsupported protocol, return as is.
    }
    else if(v.minor < Ice::currentProtocol.minor)
    {
        return v; // Supported protocol.
    }
    else
    {
        //
        // Unsupported but compatible, use the currently supported
        // protocol, that's the best we can do.
        //
        return Ice::currentProtocol;
    }
}

//
// Either return the given encoding if not compatible, or the greatest
// supported encoding otherwise.
//
inline const Ice::EncodingVersion&
getCompatibleEncoding(const Ice::EncodingVersion& v)
{
    if(v.major != Ice::currentEncoding.major)
    {
        return v; // Unsupported encoding, return as is.
    }
    else if(v.minor < Ice::currentEncoding.minor)
    {
        return v; // Supported encoding.
    }
    else
    {
        //
        // Unsupported but compatible, use the currently supported
        // encoding, that's the best we can do.
        //
        return Ice::currentEncoding;
    }
}

}

#endif
