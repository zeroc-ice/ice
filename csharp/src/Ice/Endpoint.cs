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
public abstract class EndpointInfo
{
    /// <summary>
    /// The information of the underlying endpoint or null if there's no underlying endpoint.
    /// </summary>
    public EndpointInfo? underlying;

    /// <summary>
    /// The timeout for the endpoint in milliseconds. -1 means no timeout.
    /// </summary>
    public int timeout;

    /// <summary>
    /// Specifies whether or not compression should be used if available when using this endpoint.
    /// </summary>
    public bool compress;

    /// <summary>Returns the type of the endpoint.</summary>
    /// <returns>The endpoint type.</returns>
    public abstract short type();

    /// <summary>Returns true if this endpoint is a datagram endpoint.</summary>
    /// <returns>True for a datagram endpoint.</returns>
    public abstract bool datagram();

    /// <summary>Returns true if this endpoint is secure; otherwise false.</summary>
    /// <returns>True if the endpoint is secure.</returns>
    public abstract bool secure();
}

/// <summary>
/// Provides access to the address details of a IP endpoint.
/// </summary>
public abstract class IPEndpointInfo : EndpointInfo
{
    /// <summary>
    /// The host or address configured with the endpoint.
    /// </summary>
    public string host = "";

    /// <summary>
    /// The endpoint's port number.
    /// </summary>
    public int port;

    /// <summary>
    /// The source IP address.
    /// </summary>
    public string sourceAddress = "";
}

public abstract class TCPEndpointInfo : IPEndpointInfo
{
}

/// <summary>
/// Provides access to a TCP endpoint information.
/// </summary>
public abstract class UDPEndpointInfo : IPEndpointInfo
{
    public string mcastInterface = "";

    public int mcastTtl;
}

/// <summary>
/// Provides access to a WebSocket endpoint information.
/// </summary>
public abstract class WSEndpointInfo : EndpointInfo
{
    /// <summary>
    /// The URI configured with the endpoint.
    /// </summary>
    public string resource = "";
}

/// <summary>
/// Provides access to the details of an opaque endpoint.
/// </summary>
public abstract class OpaqueEndpointInfo : EndpointInfo
{
    /// <summary>
    /// The encoding version of the opaque endpoint (to decode or encode the rawBytes).
    /// </summary>
    public EncodingVersion rawEncoding;

    /// <summary>
    /// The raw encoding of the opaque endpoint.
    /// </summary>
    public byte[] rawBytes = [];
}
