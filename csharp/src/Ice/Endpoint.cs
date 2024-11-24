// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Base class for all endpoints.
/// </summary>
public interface Endpoint
{
    /// <summary>
    /// Returns the endpoint information.
    /// </summary>
    /// <returns>The endpoint information class.</returns>
    EndpointInfo getInfo();
}

/// <summary>
/// Base class providing access to the endpoint details.
/// </summary>
public class EndpointInfo
{
    /// <summary>
    /// Gets the information for the underlying endpoint, or null if there's no underlying endpoint.
    /// </summary>
    public readonly EndpointInfo? underlying;

    /// <summary>
    /// Gets the timeout of the endpoint in milliseconds. -1 means no timeout.
    /// </summary>
    public readonly int timeout;

    /// <summary>
    /// Gets a value indicating whether or not compression should be used if available when using this endpoint.
    /// </summary>
    public readonly bool compress;

    /// <summary>Gets a 16-bit integer that identifies the transport of this endpoint.</summary>
    /// <returns>The endpoint type.</returns>
    /// <remarks>The type of an underlying endpoint is always the same as the type its enclosing endpoint.</remarks>
    public virtual short type() => underlying?.type() ?? -1;

    /// <summary>Checks if this endpoint's transport is a datagram transport such as UDP.</summary>
    /// <returns>True for a datagram endpoint; otherwise, false.</returns>
    public virtual bool datagram() => underlying?.datagram() ?? false;

    /// <summary>Checks if this endpoint's transport is secure.</summary>
    /// <returns>True if the endpoint's transport is secure; otherwise, false.</returns>
    /// <remarks>The value returned for an underlying endpoint is the same as the value returned for the enclosing
    /// endpoint.</remarks>
    public virtual bool secure() => underlying?.secure() ?? false;

    protected EndpointInfo(EndpointInfo underlying)
    {
        this.underlying = underlying;
        timeout = underlying.timeout;
        compress = underlying.compress;
    }

    protected EndpointInfo(bool compress, int timeout)
    {
        this.compress = compress;
        this.timeout = timeout;
    }
}

/// <summary>
/// Provides access to the details of an IP endpoint.
/// </summary>
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

    protected IPEndpointInfo(bool compress, int timeout, string host, int port, string sourceAddress)
        : base(compress, timeout)
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
        int timeout,
        string host,
        int port,
        string sourceAddress,
        short type,
        bool secure)
        : base(compress, timeout, host, port, sourceAddress)
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
        : base(compress, timeout: -1, host, port, sourceAddress)
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
        : base(compress: false, timeout: -1)
    {
        _type = type;
        this.rawEncoding = rawEncoding;
        this.rawBytes = rawBytes;
    }
}
