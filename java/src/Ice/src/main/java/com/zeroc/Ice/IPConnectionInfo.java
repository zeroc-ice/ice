//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * Provides access to the connection details of an IP connection
 **/
public class IPConnectionInfo extends ConnectionInfo
{
    public IPConnectionInfo()
    {
        super();
        this.localAddress = "";
        this.localPort = -1;
        this.remoteAddress = "";
        this.remotePort = -1;
    }

    public IPConnectionInfo(ConnectionInfo underlying, boolean incoming, String adapterName, String connectionId, String localAddress, int localPort, String remoteAddress, int remotePort)
    {
        super(underlying, incoming, adapterName, connectionId);
        this.localAddress = localAddress;
        this.localPort = localPort;
        this.remoteAddress = remoteAddress;
        this.remotePort = remotePort;
    }

    /**
     * The local address.
     **/
    public String localAddress;

    /**
     * The local port.
     **/
    public int localPort;

    /**
     * The remote address.
     **/
    public String remoteAddress;

    /**
     * The remote port.
     **/
    public int remotePort;

    public IPConnectionInfo clone()
    {
        return (IPConnectionInfo)super.clone();
    }

    /** @hidden */
    public static final long serialVersionUID = 8533006463792298184L;
}
