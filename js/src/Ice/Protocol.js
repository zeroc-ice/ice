// Copyright (c) ZeroC, Inc.

import { StringUtil } from "./StringUtil.js";
import { MarshalException, ParseException } from "./LocalExceptions.js";
import { Ice as Ice_Version } from "./Version.js";
const { EncodingVersion, ProtocolVersion } = Ice_Version;

export const Protocol = {};

export const Encoding_1_0 = new EncodingVersion(1, 0);
export const Encoding_1_1 = new EncodingVersion(1, 1);

export const Protocol_1_0 = new ProtocolVersion(1, 0);

//
// Size of the Ice protocol header
//
// Magic number (4 bytes)
// Protocol version major (Byte)
// Protocol version minor (Byte)
// Encoding version major (Byte)
// Encoding version minor (Byte)
// Message type (Byte)
// Compression status (Byte)
// Message size (Int)
//
Protocol.headerSize = 14;

//
// The magic number at the front of each message ['I', 'c', 'e', 'P']
//
Protocol.magic = new Uint8Array([0x49, 0x63, 0x65, 0x50]);

//
// The current Ice protocol and encoding version
//
Protocol.protocolMajor = 1;
Protocol.protocolMinor = 0;
Protocol.protocolEncodingMajor = 1;
Protocol.protocolEncodingMinor = 0;

Protocol.encodingMajor = 1;
Protocol.encodingMinor = 1;

//
// The Ice protocol message types
//
Protocol.requestMsg = 0;
Protocol.requestBatchMsg = 1;
Protocol.replyMsg = 2;
Protocol.validateConnectionMsg = 3;
Protocol.closeConnectionMsg = 4;

Protocol.requestHdr = new Uint8Array([
    Protocol.magic[0],
    Protocol.magic[1],
    Protocol.magic[2],
    Protocol.magic[3],
    Protocol.protocolMajor,
    Protocol.protocolMinor,
    Protocol.protocolEncodingMajor,
    Protocol.protocolEncodingMinor,
    Protocol.requestMsg,
    0, // Compression status.
    0,
    0,
    0,
    0, // Message size (placeholder).
    0,
    0,
    0,
    0, // Request ID (placeholder).
]);

Protocol.requestBatchHdr = new Uint8Array([
    Protocol.magic[0],
    Protocol.magic[1],
    Protocol.magic[2],
    Protocol.magic[3],
    Protocol.protocolMajor,
    Protocol.protocolMinor,
    Protocol.protocolEncodingMajor,
    Protocol.protocolEncodingMinor,
    Protocol.requestBatchMsg,
    0, // Compression status.
    0,
    0,
    0,
    0, // Message size (placeholder).
    0,
    0,
    0,
    0, // Number of requests in batch (placeholder).
]);

Protocol.replyHdr = new Uint8Array([
    Protocol.magic[0],
    Protocol.magic[1],
    Protocol.magic[2],
    Protocol.magic[3],
    Protocol.protocolMajor,
    Protocol.protocolMinor,
    Protocol.protocolEncodingMajor,
    Protocol.protocolEncodingMinor,
    Protocol.replyMsg,
    0, // Compression status.
    0,
    0,
    0,
    0, // Message size (placeholder).
]);

Protocol.currentProtocol = new ProtocolVersion(Protocol.protocolMajor, Protocol.protocolMinor);
Protocol.currentProtocolEncoding = new EncodingVersion(Protocol.protocolEncodingMajor, Protocol.protocolEncodingMinor);

Protocol.currentEncoding = new EncodingVersion(Protocol.encodingMajor, Protocol.encodingMinor);

Protocol.checkSupportedEncoding = function (v) {
    if (v.major !== Protocol.currentEncoding.major || v.minor > Protocol.currentEncoding.minor) {
        throw new MarshalException(`This Ice runtime does not support encoding version ${v.major}.${v.minor}`);
    }
};

//
// Either return the given protocol if not compatible, or the greatest
// supported protocol otherwise.
//
Protocol.getCompatibleProtocol = function (v) {
    if (v.major !== Protocol.currentProtocol.major) {
        return v; // Unsupported protocol, return as is.
    } else if (v.minor < Protocol.currentProtocol.minor) {
        return v; // Supported protocol.
    } else {
        //
        // Unsupported but compatible, use the currently supported
        // protocol, that's the best we can do.
        //
        return Protocol.currentProtocol;
    }
};

//
// Either return the given encoding if not compatible, or the greatest
// supported encoding otherwise.
//
Protocol.getCompatibleEncoding = function (v) {
    if (v.major !== Protocol.currentEncoding.major) {
        return v; // Unsupported encoding, return as is.
    } else if (v.minor < Protocol.currentEncoding.minor) {
        return v; // Supported encoding.
    } else {
        //
        // Unsupported but compatible, use the currently supported
        // encoding, that's the best we can do.
        //
        return Protocol.currentEncoding;
    }
};

Protocol.isSupported = function (version, supported) {
    return version.major === supported.major && version.minor <= supported.minor;
};

/**
 * Converts a string to a protocol version.
 *
 * @param version The string to convert.
 *
 * @return The converted protocol version.
 **/
export function stringToProtocolVersion(version) {
    return new ProtocolVersion(stringToMajor(version), stringToMinor(version));
}

/**
 * Converts a string to an encoding version.
 *
 * @param version The string to convert.
 *
 * @return The converted object identity.
 **/
export function stringToEncodingVersion(version) {
    return new EncodingVersion(stringToMajor(version), stringToMinor(version));
}

/**
 * Converts a protocol version to a string.
 *
 * @param v The protocol version to convert.
 *
 * @return The converted string.
 **/
export function protocolVersionToString(v) {
    return majorMinorToString(v.major, v.minor);
}

/**
 * Converts an encoding version to a string.
 *
 * @param v The encoding version to convert.
 *
 * @return The converted string.
 **/
export function encodingVersionToString(v) {
    return majorMinorToString(v.major, v.minor);
}

Protocol.OPTIONAL_END_MARKER = 0xff;
Protocol.FLAG_HAS_TYPE_ID_STRING = 1 << 0;
Protocol.FLAG_HAS_TYPE_ID_INDEX = 1 << 1;
Protocol.FLAG_HAS_TYPE_ID_COMPACT = (1 << 1) | (1 << 0);
Protocol.FLAG_HAS_OPTIONAL_MEMBERS = 1 << 2;
Protocol.FLAG_HAS_INDIRECTION_TABLE = 1 << 3;
Protocol.FLAG_HAS_SLICE_SIZE = 1 << 4;
Protocol.FLAG_IS_LAST_SLICE = 1 << 5;

function stringToMajor(str) {
    const pos = str.indexOf(".");
    if (pos === -1) {
        throw new ParseException(`malformed version value in '${str}'`);
    }

    try {
        const majVersion = StringUtil.toInt32(str.substring(0, pos));
        if (majVersion < 1 || majVersion > 255) {
            throw new ParseException(`range error in version ${str}'`);
        }
        return majVersion;
    } catch (ex) {
        throw new ParseException(`invalid version value in '${str}'`, ex);
    }
}

function stringToMinor(str) {
    const pos = str.indexOf(".");
    if (pos === -1) {
        throw new ParseException(`malformed version value in '${str}'`);
    }

    try {
        const minVersion = StringUtil.toInt32(str.substring(pos + 1));
        if (minVersion < 0 || minVersion > 255) {
            throw new ParseException(`range error in version '${str}'`);
        }
        return minVersion;
    } catch (ex) {
        throw new ParseException(`invalid version value in '${str}'`, ex);
    }
}

function majorMinorToString(major, minor) {
    return major + "." + minor;
}
