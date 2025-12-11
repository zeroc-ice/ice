// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// An endpoint specifies the address of the server-end of an Ice connection: an object adapter listens on one or
/// more endpoints and a client establishes a connection to an endpoint.
/// </summary>
public interface Endpoint
{
    /// <summary>
    /// Returns this endpoint's information.
    /// </summary>
    /// <returns>This endpoint's information class.</returns>
    EndpointInfo getInfo();
}

/// <summary>
/// Base class for the endpoint info classes.
/// </summary>
public class EndpointInfo
{
    /// <summary>
    /// The information of the underlying endpoint or null if there's no underlying endpoint.
    /// </summary>
    public readonly EndpointInfo? underlying;

    /// <summary>
    /// Specifies whether or not compression should be used if available when using this endpoint.
    /// </summary>
    public readonly bool compress;

    /// <summary>
    /// Returns the type of the endpoint.
    /// </summary>
    /// <returns>The endpoint type.</returns>
    public virtual short type() => underlying?.type() ?? -1;

    /// <summary>
    /// Returns <see langword="true"/> if this endpoint's transport is a datagram transport (namely, UDP),
    /// <see langword="false"/> otherwise.
    /// </summary>
    /// <returns><see langword="true"/> for a UDP endpoint, <see langword="false"/> otherwise.</returns>
    public virtual bool datagram() => underlying?.datagram() ?? false;

    /// <summary>
    /// Returns <see langword="true"/> if this endpoint's transport uses SSL, <see langword="false"/> otherwise.
    /// </summary>
    /// <returns><see langword="true"/> for SSL and SSL-based transports, <see langword="false"/> otherwise.</returns>
    public virtual bool secure() => underlying?.secure() ?? false;

    protected EndpointInfo(EndpointInfo underlying)
    {
        this.underlying = underlying;
        compress = underlying.compress;
    }

    protected EndpointInfo(bool compress) => this.compress = compress;
}

/// <summary>
/// Provides access to the address details of an IP endpoint.
/// </summary>
/// <seealso cref="Endpoint"/>
public class IPEndpointInfo : EndpointInfo
{
    /// <summary>
    /// Gets the host or address configured with the endpoint.
    /// </summary>
    public readonly string host;

    /// <summary>
    /// Gets the endpoint's port number.
    /// </summary>
    public readonly int port;

    /// <summary>
    /// Gets the source IP address.
    /// </summary>
    public readonly string sourceAddress;

    protected IPEndpointInfo(bool compress, string host, int port, string sourceAddress)
        : base(compress)
    {
        this.host = host;
        this.port = port;
        this.sourceAddress = sourceAddress;
    }
}

public sealed class TCPEndpointInfo : IPEndpointInfo
{
    private readonly bool _secure;
    private readonly short _type;

    public override short type() => _type;

    public override bool secure() => _secure;

    internal TCPEndpointInfo(
        bool compress,
        string host,
        int port,
        string sourceAddress,
        short type,
        bool secure)
        : base(compress, host, port, sourceAddress)
    {
        _type = type;
        _secure = secure;
    }
}

/// <summary>
/// Provides access to a UDP endpoint information.
/// </summary>
public sealed class UDPEndpointInfo : IPEndpointInfo
{
    public readonly string mcastInterface;

    public readonly int mcastTtl;

    public override short type() => UDPEndpointType.value;

    public override bool datagram() => true;

    internal UDPEndpointInfo(
        bool compress,
        string host,
        int port,
        string sourceAddress,
        string mcastInterface,
        int mcastTtl)
        : base(compress, host, port, sourceAddress)
    {
        this.mcastInterface = mcastInterface;
        this.mcastTtl = mcastTtl;
    }
}

/// <summary>
/// Provides access to a WebSocket endpoint information.
/// </summary>
public sealed class WSEndpointInfo : EndpointInfo
{
    /// <summary>
    /// Gets the URI configured for this endpoint.
    /// </summary>
    public readonly string resource;

    internal WSEndpointInfo(EndpointInfo underlying, string resource)
        : base(underlying) => this.resource = resource;
}

/// <summary>
/// Provides access to the details of an opaque endpoint.
/// </summary>
public sealed class OpaqueEndpointInfo : EndpointInfo
{
    /// <summary>
    /// Gets the raw encoding (to decode the rawBytes).
    /// </summary>
    public readonly EncodingVersion rawEncoding;

    /// <summary>
    /// Gets the raw bytes of the opaque endpoint.
    /// </summary>
    public readonly byte[] rawBytes;

    private readonly short _type;

    public override short type() => _type;

    internal OpaqueEndpointInfo(short type, EncodingVersion rawEncoding, byte[] rawBytes)
        : base(compress: false)
    {
        _type = type;
        this.rawEncoding = rawEncoding;
        this.rawBytes = rawBytes;
    }
}
