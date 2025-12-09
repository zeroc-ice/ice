// Copyright (c) ZeroC, Inc.

using System.Globalization;

namespace Ice.Internal;

public sealed class Protocol
{
    /// <summary>
    /// Converts a string to a protocol version.
    /// </summary>
    /// <param name="version">The string to convert.</param>
    /// <returns>The converted protocol version.</returns>
    internal static ProtocolVersion stringToProtocolVersion(string version)
    {
        stringToMajorMinor(version, out byte major, out byte minor);
        return new ProtocolVersion(major, minor);
    }

    /// <summary>
    /// Converts a string to an encoding version.
    /// </summary>
    /// <param name="version">The string to convert.</param>
    /// <returns>The converted encoding version.</returns>
    internal static EncodingVersion stringToEncodingVersion(string version)
    {
        stringToMajorMinor(version, out byte major, out byte minor);
        return new EncodingVersion(major, minor);
    }

    /// <summary>
    /// Converts a protocol version to a string.
    /// </summary>
    /// <param name="v">The protocol version to convert.</param>
    /// <returns>The converted string.</returns>
    internal static string protocolVersionToString(Ice.ProtocolVersion v) => majorMinorToString(v.major, v.minor);

    /// <summary>
    /// Converts an encoding version to a string.
    /// </summary>
    /// <param name="v">The encoding version to convert.</param>
    /// <returns>The converted string.</returns>
    internal static string encodingVersionToString(Ice.EncodingVersion v) => majorMinorToString(v.major, v.minor);

    internal static readonly ProtocolVersion Protocol_1_0 = new(1, 0);

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
        if (v.major != Ice.Util.Encoding_1_1.major || v.minor > Ice.Util.Encoding_1_1.minor)
        {
            throw new MarshalException(
                $"This Ice runtime does not support encoding version {v.major}.{v.minor}");
        }
    }

    internal static bool
    isSupported(Ice.EncodingVersion version, Ice.EncodingVersion supported) =>
        version.major == supported.major && version.minor <= supported.minor;

    private static void stringToMajorMinor(string str, out byte major, out byte minor)
    {
        int pos = str.IndexOf('.', StringComparison.Ordinal);
        if (pos == -1)
        {
            throw new ParseException($"malformed version value in '{str}'");
        }

        string majStr = str[..pos];
        string minStr = str[(pos + 1)..];
        int majVersion;
        int minVersion;
        try
        {
            majVersion = int.Parse(majStr, CultureInfo.InvariantCulture);
            minVersion = int.Parse(minStr, CultureInfo.InvariantCulture);
        }
        catch (FormatException ex)
        {
            throw new ParseException($"invalid version value in '{str}'", ex);
        }

        if (majVersion < 1 || majVersion > 255 || minVersion < 0 || minVersion > 255)
        {
            throw new ParseException($"range error in version '{str}'");
        }

        major = (byte)majVersion;
        minor = (byte)minVersion;
    }

    private static string majorMinorToString(byte major, byte minor) => $"{major}.{minor}";

    private Protocol()
    {
    }
}
