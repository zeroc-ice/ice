// Copyright (c) ZeroC, Inc.

namespace Ice.Internal;

public sealed class Protocol
{
    internal static readonly ProtocolVersion currentProtocol = new(protocolMajor, protocolMinor);

    internal static readonly EncodingVersion currentProtocolEncoding =
        new(protocolEncodingMajor, protocolEncodingMinor);

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
    internal const int headerSize = 14;

    //
    // The magic number at the front of each message
    //
    internal static readonly byte[] magic = [0x49, 0x63, 0x65, 0x50]; // 'I', 'c', 'e', 'P'

    //
    // The current Ice protocol and Slice encoding version
    //
    internal const byte protocolMajor = 1;
    internal const byte protocolMinor = 0;
    internal const byte protocolEncodingMajor = 1;
    internal const byte protocolEncodingMinor = 0;

    public const byte OPTIONAL_END_MARKER = 0xFF;

    public const byte FLAG_HAS_TYPE_ID_STRING = 1 << 0;
    public const byte FLAG_HAS_TYPE_ID_INDEX = 1 << 1;
    public const byte FLAG_HAS_TYPE_ID_COMPACT = (1 << 1) | (1 << 0);
    public const byte FLAG_HAS_OPTIONAL_MEMBERS = 1 << 2;
    public const byte FLAG_HAS_INDIRECTION_TABLE = 1 << 3;
    public const byte FLAG_HAS_SLICE_SIZE = 1 << 4;
    public const byte FLAG_IS_LAST_SLICE = 1 << 5;

    //
    // The Ice protocol message types
    //
    internal const byte requestMsg = 0;
    internal const byte requestBatchMsg = 1;
    internal const byte replyMsg = 2;
    internal const byte validateConnectionMsg = 3;
    internal const byte closeConnectionMsg = 4;

    internal static readonly byte[] requestHdr = new byte[]
    {
        magic[0], magic[1], magic[2], magic[3],
        protocolMajor, protocolMinor,
        protocolEncodingMajor, protocolEncodingMinor,
        requestMsg,
        0, // Compression status.
        0, 0, 0, 0, // Message size (placeholder).
        0, 0, 0, 0  // Request ID (placeholder).
    };

    internal static readonly byte[] requestBatchHdr = new byte[]
    {
        magic[0], magic[1], magic[2], magic[3],
        protocolMajor, protocolMinor,
        protocolEncodingMajor, protocolEncodingMinor,
        requestBatchMsg,
        0, // Compression status.
        0, 0, 0, 0, // Message size (placeholder).
        0, 0, 0, 0  // Number of requests in batch (placeholder).
    };

    internal static readonly byte[] replyHdr = new byte[]
    {
        magic[0], magic[1], magic[2], magic[3],
        protocolMajor, protocolMinor,
        protocolEncodingMajor, protocolEncodingMinor,
        replyMsg,
        0, // Compression status.
        0, 0, 0, 0 // Message size (placeholder).
    };

    internal static void checkSupportedEncoding(Ice.EncodingVersion v)
    {
        if (v.major != Util.Encoding_1_1.major || v.minor > Util.Encoding_1_1.minor)
        {
            throw new MarshalException(
                $"This Ice runtime does not support encoding version {v.major}.{v.minor}");
        }
    }

    internal static bool
    isSupported(Ice.EncodingVersion version, Ice.EncodingVersion supported) =>
        version.major == supported.major && version.minor <= supported.minor;

    private Protocol()
    {
    }
}
