// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

final class Protocol {
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
    public static final int headerSize = 14;

    //
    // The magic number at the front of each message
    //
    public static final byte[] magic = {0x49, 0x63, 0x65, 0x50}; // 'I', 'c', 'e', 'P'

    //
    // The current Ice protocol and encoding version
    //
    public static final byte protocolMajor = 1;
    public static final byte protocolMinor = 0;
    public static final byte protocolEncodingMajor = 1;
    public static final byte protocolEncodingMinor = 0;

    public static final byte encodingMajor = 1;
    public static final byte encodingMinor = 1;

    //
    // The Ice protocol message types
    //
    public static final byte requestMsg = 0;
    public static final byte requestBatchMsg = 1;
    public static final byte replyMsg = 2;
    public static final byte validateConnectionMsg = 3;
    public static final byte closeConnectionMsg = 4;

    public static final byte[] requestHdr = {
        Protocol.magic[0],
        Protocol.magic[1],
        Protocol.magic[2],
        Protocol.magic[3],
        Protocol.protocolMajor,
        Protocol.protocolMinor,
        Protocol.protocolEncodingMajor,
        Protocol.protocolEncodingMinor,
        Protocol.requestMsg,
        (byte) 0, // Compression status.
        (byte) 0,
        (byte) 0,
        (byte) 0,
        (byte) 0, // Message size (placeholder).
        (byte) 0,
        (byte) 0,
        (byte) 0,
        (byte) 0 // Request ID (placeholder).
    };

    public static final byte[] requestBatchHdr = {
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
        (byte) 0,
        (byte) 0,
        (byte) 0,
        (byte) 0, // Message size (placeholder).
        (byte) 0,
        (byte) 0,
        (byte) 0,
        (byte) 0 // Number of requests in batch (placeholder).
    };

    public static final byte[] replyHdr = {
        Protocol.magic[0],
        Protocol.magic[1],
        Protocol.magic[2],
        Protocol.magic[3],
        Protocol.protocolMajor,
        Protocol.protocolMinor,
        Protocol.protocolEncodingMajor,
        Protocol.protocolEncodingMinor,
        Protocol.replyMsg,
        (byte) 0, // Compression status.
        (byte) 0,
        (byte) 0,
        (byte) 0,
        (byte) 0 // Message size (placeholder).
    };

    public static final ProtocolVersion currentProtocol =
        new ProtocolVersion(protocolMajor, protocolMinor);
    public static final EncodingVersion currentProtocolEncoding =
        new EncodingVersion(protocolEncodingMajor, protocolEncodingMinor);

    public static final EncodingVersion currentEncoding =
        new EncodingVersion(encodingMajor, encodingMinor);

    public static void checkSupportedProtocol(ProtocolVersion v) {
        if (v.major != currentProtocol.major || v.minor > currentProtocol.minor) {
            throw new FeatureNotSupportedException(
                "Cannot send request using protocol version " + v.major + "." + v.minor);
        }
    }

    public static void checkSupportedProtocolEncoding(EncodingVersion v) {
        if (v.major != currentProtocolEncoding.major || v.minor > currentProtocolEncoding.minor) {
            throw new MarshalException(
                "This Ice runtime does not support encoding version "
                    + v.major
                    + "."
                    + v.minor);
        }
    }

    public static void checkSupportedEncoding(EncodingVersion v) {
        if (v.major != currentEncoding.major || v.minor > currentEncoding.minor) {
            throw new MarshalException(
                "This Ice runtime does not support encoding version "
                    + v.major
                    + "."
                    + v.minor);
        }
    }

    //
    // Either return the given protocol if not compatible, or the greatest
    // supported protocol otherwise.
    //
    public static ProtocolVersion getCompatibleProtocol(ProtocolVersion v) {
        if (v.major != currentProtocol.major) {
            return v; // Unsupported protocol, return as is.
        } else if (v.minor < currentProtocol.minor) {
            return v; // Supported protocol.
        } else {
            //
            // Unsupported but compatible, use the currently supported
            // protocol, that's the best we can do.
            //
            return currentProtocol;
        }
    }

    //
    // Either return the given encoding if not compatible, or the greatest
    // supported encoding otherwise.
    //
    public static EncodingVersion getCompatibleEncoding(EncodingVersion v) {
        if (v.major != currentEncoding.major) {
            return v; // Unsupported encoding, return as is.
        } else if (v.minor < currentEncoding.minor) {
            return v; // Supported encoding.
        } else {
            //
            // Unsupported but compatible, use the currently supported
            // encoding, that's the best we can do.
            //
            return currentEncoding;
        }
    }

    public static boolean isSupported(EncodingVersion version, EncodingVersion supported) {
        return version.major == supported.major && version.minor <= supported.minor;
    }

    public static final int OPTIONAL_END_MARKER = 0xFF;

    public static final byte FLAG_HAS_TYPE_ID_STRING = (byte) (1 << 0);
    public static final byte FLAG_HAS_TYPE_ID_INDEX = (byte) (1 << 1);
    public static final byte FLAG_HAS_TYPE_ID_COMPACT = (byte) (1 << 1 | 1 << 0);
    public static final byte FLAG_HAS_OPTIONAL_MEMBERS = (byte) (1 << 2);
    public static final byte FLAG_HAS_INDIRECTION_TABLE = (byte) (1 << 3);
    public static final byte FLAG_HAS_SLICE_SIZE = (byte) (1 << 4);
    public static final byte FLAG_IS_LAST_SLICE = (byte) (1 << 5);

    private Protocol() {}
}
