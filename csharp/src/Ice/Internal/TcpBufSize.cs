// Copyright (c) ZeroC, Inc.

namespace Ice.Internal;

/// <summary>
/// The TCP send and receive buffer sizes, as configured by the Ice.TCP.SndSize and Ice.TCP.RcvSize properties.
/// </summary>
internal sealed class TcpBufSize
{
    internal int rcvSize { get; }

    internal int sndSize { get; }

    /// <summary>
    /// Initializes a new instance of the <see cref="TcpBufSize" /> class by reading the Ice.TCP.RcvSize and
    /// Ice.TCP.SndSize properties. Throws PropertyException if a value is not a valid integer.
    /// </summary>
    internal TcpBufSize(Properties properties)
    {
        // By default, on Windows we use a 128KB buffer size. On Unix platforms, we use the system defaults.
        int dfltBufSize = AssemblyUtil.isWindows ? 128 * 1024 : 0;

        rcvSize = properties.getPropertyAsIntWithDefault("Ice.TCP.RcvSize", dfltBufSize);
        sndSize = properties.getPropertyAsIntWithDefault("Ice.TCP.SndSize", dfltBufSize);
    }
}
