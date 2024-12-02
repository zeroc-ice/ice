// Copyright (c) ZeroC, Inc.

package com.zeroc.IceBT;

/** Provides access to the details of a Bluetooth connection. */
public final class ConnectionInfo extends com.zeroc.Ice.ConnectionInfo {
    /** The local Bluetooth address. */
    public final String localAddress;

    /** The local RFCOMM channel. */
    public final int localChannel;

    /** The remote Bluetooth address. */
    public final String remoteAddress;

    /** The remote RFCOMM channel. */
    public final int remoteChannel;

    /** The UUID of the service being offered (in a server) or targeted (in a client). */
    public final String uuid;

    /** The connection buffer receive size. */
    public final int rcvSize;

    /** The connection buffer send size. */
    public final int sndSize;

    ConnectionInfo(
            boolean incoming,
            String adapterName,
            String connectionId,
            String localAddress,
            int localChannel,
            String remoteAddress,
            int remoteChannel,
            String uuid,
            int rcvSize,
            int sndSize) {
        super(incoming, adapterName, connectionId);
        this.localAddress = localAddress;
        this.localChannel = localChannel;
        this.remoteAddress = remoteAddress;
        this.remoteChannel = remoteChannel;
        this.uuid = uuid;
        this.rcvSize = rcvSize;
        this.sndSize = sndSize;
    }
}
