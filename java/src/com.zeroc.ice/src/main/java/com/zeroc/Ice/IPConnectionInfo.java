// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Provides access to the connection details of an IP connection */
public class IPConnectionInfo extends ConnectionInfo {
    /** The local address. */
    public final String localAddress;

    /** The local port. */
    public final int localPort;

    /** The remote address. */
    public final String remoteAddress;

    /** The remote port. */
    public final int remotePort;

    /**
     * Constructs an IPConnectionInfo with the specified connection parameters.
     *
     * @param incoming whether this connection is incoming
     * @param adapterName the name of the adapter associated with this connection
     * @param connectionId the ID of this connection
     * @param localAddress the local IP address of this connection
     * @param localPort the local port number of this connection
     * @param remoteAddress the remote IP address of this connection
     * @param remotePort the remote port number of this connection
     */
    protected IPConnectionInfo(
            boolean incoming,
            String adapterName,
            String connectionId,
            String localAddress,
            int localPort,
            String remoteAddress,
            int remotePort) {
        super(incoming, adapterName, connectionId);
        this.localAddress = localAddress;
        this.localPort = localPort;
        this.remoteAddress = remoteAddress;
        this.remotePort = remotePort;
    }
}
