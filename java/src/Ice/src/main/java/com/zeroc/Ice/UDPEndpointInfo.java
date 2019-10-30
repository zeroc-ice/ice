//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * Provides access to an UDP endpoint information.
 *
 * @see Endpoint
 **/
public abstract class UDPEndpointInfo extends IPEndpointInfo
{
    public UDPEndpointInfo()
    {
        super();
        this.mcastInterface = "";
    }

    public UDPEndpointInfo(EndpointInfo underlying, int timeout, boolean compress, String host, int port, String sourceAddress, String mcastInterface, int mcastTtl)
    {
        super(underlying, timeout, compress, host, port, sourceAddress);
        this.mcastInterface = mcastInterface;
        this.mcastTtl = mcastTtl;
    }

    /**
     * The multicast interface.
     **/
    public String mcastInterface;

    /**
     * The multicast time-to-live (or hops).
     **/
    public int mcastTtl;

    public UDPEndpointInfo clone()
    {
        return (UDPEndpointInfo)super.clone();
    }

    /** @hidden */
    public static final long serialVersionUID = 6545930812316183136L;
}
