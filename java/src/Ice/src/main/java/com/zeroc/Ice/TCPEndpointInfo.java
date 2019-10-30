//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * Provides access to a TCP endpoint information.
 *
 * @see Endpoint
 **/
public abstract class TCPEndpointInfo extends IPEndpointInfo
{
    public TCPEndpointInfo()
    {
        super();
    }

    public TCPEndpointInfo(EndpointInfo underlying, int timeout, boolean compress, String host, int port, String sourceAddress)
    {
        super(underlying, timeout, compress, host, port, sourceAddress);
    }

    public TCPEndpointInfo clone()
    {
        return (TCPEndpointInfo)super.clone();
    }

    /** @hidden */
    public static final long serialVersionUID = -7607491251938374557L;
}
