// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Provides access to the connection details of a TCP connection */
public final class TCPConnectionInfo extends IPConnectionInfo {
    /** The connection buffer receive size. */
    public final int rcvSize;

    /** The connection buffer send size. */
    public final int sndSize;

    TCPConnectionInfo(
            boolean incoming,
            String adapterName,
            String connectionId,
            String localAddress,
            int localPort,
            String remoteAddress,
            int remotePort,
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
        this.rcvSize = rcvSize;
        this.sndSize = sndSize;
    }

    TCPConnectionInfo(boolean incoming, String adapterName, String connectionId) {
        this(incoming, adapterName, connectionId, "", -1, "", -1, 0, 0);
    }
}
