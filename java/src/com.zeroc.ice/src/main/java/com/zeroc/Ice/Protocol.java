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
    // The current Ice protocol and Slice encoding version
    //
    public static final byte protocolMajor = 1;
    public static final byte protocolMinor = 0;
    public static final byte protocolEncodingMajor = 1;
    public static final byte protocolEncodingMinor = 0;

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

    public static final ProtocolVersion currentProtocol = new ProtocolVersion(protocolMajor, protocolMinor);
    public static final EncodingVersion currentProtocolEncoding =
        new EncodingVersion(protocolEncodingMajor, protocolEncodingMinor);

    public static void checkSupportedProtocol(ProtocolVersion v) {
        if (!v.equals(currentProtocol)) {
            throw new FeatureNotSupportedException(
                "Cannot send request using protocol version " + v.major + "." + v.minor);
        }
    }

    public static void checkSupportedProtocolEncoding(EncodingVersion v) {
        if (!v.equals(currentProtocolEncoding)) {
            throw new MarshalException("This Ice runtime does not support encoding version " + v.major + "." + v.minor);
        }
    }

    public static void checkSupportedEncoding(EncodingVersion v) {
        if (v.major != Util.Encoding_1_1.major || v.minor > Util.Encoding_1_1.minor) {
            throw new MarshalException("This Ice runtime does not support encoding version " + v.major + "." + v.minor);
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
