//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceBT;

/**
 * Provides access to the details of a Bluetooth connection.
 **/
public class ConnectionInfo extends com.zeroc.Ice.ConnectionInfo
{
    public ConnectionInfo()
    {
        super();
        this.localAddress = "";
        this.localChannel = -1;
        this.remoteAddress = "";
        this.remoteChannel = -1;
        this.uuid = "";
        this.rcvSize = 0;
        this.sndSize = 0;
    }

    public ConnectionInfo(com.zeroc.Ice.ConnectionInfo underlying, boolean incoming, String adapterName, String connectionId, String localAddress, int localChannel, String remoteAddress, int remoteChannel, String uuid, int rcvSize, int sndSize)
    {
        super(underlying, incoming, adapterName, connectionId);
        this.localAddress = localAddress;
        this.localChannel = localChannel;
        this.remoteAddress = remoteAddress;
        this.remoteChannel = remoteChannel;
        this.uuid = uuid;
        this.rcvSize = rcvSize;
        this.sndSize = sndSize;
    }

    /**
     * The local Bluetooth address.
     **/
    public String localAddress;

    /**
     * The local RFCOMM channel.
     **/
    public int localChannel;

    /**
     * The remote Bluetooth address.
     **/
    public String remoteAddress;

    /**
     * The remote RFCOMM channel.
     **/
    public int remoteChannel;

    /**
     * The UUID of the service being offered (in a server) or targeted (in a client).
     **/
    public String uuid;

    /**
     * The connection buffer receive size.
     **/
    public int rcvSize;

    /**
     * The connection buffer send size.
     **/
    public int sndSize;

    public ConnectionInfo clone()
    {
        return (ConnectionInfo)super.clone();
    }

    /** @hidden */
    public static final long serialVersionUID = -1320122761309365325L;
}
