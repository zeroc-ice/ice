//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * Provides access to the connection details of a TCP connection
 **/
public class TCPConnectionInfo extends IPConnectionInfo
{
    public TCPConnectionInfo()
    {
        super();
        this.rcvSize = 0;
        this.sndSize = 0;
    }

    public TCPConnectionInfo(ConnectionInfo underlying, boolean incoming, String adapterName, String connectionId, String localAddress, int localPort, String remoteAddress, int remotePort, int rcvSize, int sndSize)
    {
        super(underlying, incoming, adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort);
        this.rcvSize = rcvSize;
        this.sndSize = sndSize;
    }

    /**
     * The connection buffer receive size.
     **/
    public int rcvSize;

    /**
     * The connection buffer send size.
     **/
    public int sndSize;

    public TCPConnectionInfo clone()
    {
        return (TCPConnectionInfo)super.clone();
    }

    /** @hidden */
    public static final long serialVersionUID = 991962771996026865L;
}
