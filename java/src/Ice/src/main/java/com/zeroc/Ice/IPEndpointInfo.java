//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * Provides access to the address details of a IP endpoint.
 *
 * @see Endpoint
 **/
public abstract class IPEndpointInfo extends EndpointInfo
{
    public IPEndpointInfo()
    {
        super();
        this.host = "";
        this.sourceAddress = "";
    }

    public IPEndpointInfo(EndpointInfo underlying, int timeout, boolean compress, String host, int port, String sourceAddress)
    {
        super(underlying, timeout, compress);
        this.host = host;
        this.port = port;
        this.sourceAddress = sourceAddress;
    }

    /**
     * The host or address configured with the endpoint.
     **/
    public String host;

    /**
     * The port number.
     **/
    public int port;

    /**
     * The source IP address.
     **/
    public String sourceAddress;

    public IPEndpointInfo clone()
    {
        return (IPEndpointInfo)super.clone();
    }

    /** @hidden */
    public static final long serialVersionUID = 1740540942649122045L;
}
