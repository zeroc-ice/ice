//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * Provides access to the connection details of a UDP connection
 **/
public class UDPConnectionInfo extends IPConnectionInfo
{
    public UDPConnectionInfo()
    {
        super();
        this.mcastAddress = "";
        this.mcastPort = -1;
        this.rcvSize = 0;
        this.sndSize = 0;
    }

    public UDPConnectionInfo(ConnectionInfo underlying, boolean incoming, String adapterName, String connectionId, String localAddress, int localPort, String remoteAddress, int remotePort, String mcastAddress, int mcastPort, int rcvSize, int sndSize)
    {
        super(underlying, incoming, adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort);
        this.mcastAddress = mcastAddress;
        this.mcastPort = mcastPort;
        this.rcvSize = rcvSize;
        this.sndSize = sndSize;
    }

    /**
     * The multicast address.
     **/
    public String mcastAddress;

    /**
     * The multicast port.
     **/
    public int mcastPort;

    /**
     * The connection buffer receive size.
     **/
    public int rcvSize;

    /**
     * The connection buffer send size.
     **/
    public int sndSize;

    public UDPConnectionInfo clone()
    {
        return (UDPConnectionInfo)super.clone();
    }

    /** @hidden */
    public static final long serialVersionUID = -8596817245040015144L;
}
