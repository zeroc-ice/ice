// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

public abstract class EndpointInfo
{
    public EndpointInfo? underlying;
    public int timeout;
    public bool compress;

    /// <summary>Returns the type of the endpoint.</summary>
    /// <returns>The endpoint type.</returns>
    public abstract short type();

    /// <summary>Returns true if this endpoint is a datagram endpoint.</summary>
    /// <returns>True for a datagram endpoint.</returns>
    public abstract bool datagram();

    public abstract bool secure();

    protected EndpointInfo()
    {
    }

    protected EndpointInfo(EndpointInfo? underlying, int timeout, bool compress)
    {
        this.underlying = underlying;
        this.timeout = timeout;
        this.compress = compress;
    }
}

public interface Endpoint
{
    /// <summary>
    /// Returns the endpoint information.
    /// </summary>
    /// <returns>The endpoint information class.</returns>
    EndpointInfo getInfo();
}

public abstract class IPEndpointInfo : EndpointInfo
{
    public string host;

    public int port;

    public string sourceAddress;

    protected IPEndpointInfo() : base()
    {
        this.host = "";
        this.sourceAddress = "";
    }

    protected IPEndpointInfo(EndpointInfo underlying, int timeout, bool compress, string host, int port, string sourceAddress)
        : base(underlying, timeout, compress)
    {
        this.host = host;
        this.port = port;
        this.sourceAddress = sourceAddress;
    }
}

public abstract class TCPEndpointInfo : IPEndpointInfo
{
    protected TCPEndpointInfo() : base()
    {
    }

    protected TCPEndpointInfo(EndpointInfo underlying, int timeout, bool compress, string host, int port, string sourceAddress)
        : base(underlying, timeout, compress, host, port, sourceAddress)
    {
    }
}

public abstract class UDPEndpointInfo : IPEndpointInfo
{
    public string mcastInterface;

    public int mcastTtl;

    protected UDPEndpointInfo() : base()
    {
        this.mcastInterface = "";
    }

    protected UDPEndpointInfo(EndpointInfo underlying, int timeout, bool compress, string host, int port, string sourceAddress, string mcastInterface, int mcastTtl)
        : base(underlying, timeout, compress, host, port, sourceAddress)
    {
        this.mcastInterface = mcastInterface;
        this.mcastTtl = mcastTtl;
    }
}

public abstract class WSEndpointInfo : EndpointInfo
{
    public string resource;

    protected WSEndpointInfo() : base()
    {
        resource = "";
    }

    protected WSEndpointInfo(EndpointInfo underlying, int timeout, bool compress, string resource) : base(underlying, timeout, compress)
    {
        this.resource = resource;
    }
}

public abstract class OpaqueEndpointInfo : EndpointInfo
{
    public EncodingVersion rawEncoding;

    public byte[] rawBytes;

    protected OpaqueEndpointInfo() : base()
    {
        rawEncoding = new EncodingVersion();
        rawBytes = [];
    }

    protected OpaqueEndpointInfo(EndpointInfo underlying, int timeout, bool compress, EncodingVersion rawEncoding, byte[] rawBytes)
        : base(underlying, timeout, compress)
    {
        this.rawEncoding = rawEncoding;
        this.rawBytes = rawBytes;
    }
}
