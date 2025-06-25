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

    /**
     * Constructs a ConnectionInfo with the specified connection parameters.
     *
     * @param incoming whether this connection is incoming
     * @param adapterName the name of the adapter associated with this connection
     * @param connectionId the ID of this connection
     * @param localAddress the local Bluetooth address of this connection
     * @param localChannel the local RFCOMM channel of this connection
     * @param remoteAddress the remote Bluetooth address of this connection
     * @param remoteChannel the remote RFCOMM channel of this connection
     * @param uuid the UUID of the service being offered or targeted
     * @param rcvSize the receive buffer size for this connection
     * @param sndSize the send buffer size for this connection
     */
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
