// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Provides access to the connection details of a UDP connection. */
public final class UDPConnectionInfo extends IPConnectionInfo {
    /** The multicast address. */
    public final String mcastAddress;

    /** The multicast port. */
    public final int mcastPort;

    /** The size of the receive buffer. */
    public final int rcvSize;

    /** The size of the send buffer. */
    public final int sndSize;

    UDPConnectionInfo(
            boolean incoming,
            String adapterName,
            String connectionId,
            String localAddress,
            int localPort,
            String remoteAddress,
            int remotePort,
            String mcastAddress,
            int mcastPort,
            int rcvSize,
            int sndSize) {
        super(
            incoming,
            adapterName,
            connectionId,
            localAddress,
            localPort,
            remoteAddress,
            remotePort);
        this.mcastAddress = mcastAddress;
        this.mcastPort = mcastPort;
        this.rcvSize = rcvSize;
        this.sndSize = sndSize;
    }

    UDPConnectionInfo(boolean incoming, String adapterName, String connectionId) {
        this(incoming, adapterName, connectionId, "", -1, "", -1, "", -1, 0, 0);
    }
}
