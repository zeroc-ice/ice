// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Base class for all connection info classes. */
public class ConnectionInfo {
    /** The information of the underlying transport or {@code null} if there's no underlying transport. */
    public final ConnectionInfo underlying;

    /** {@code true} if this is an incoming connection, {@code false} otherwise. */
    public final boolean incoming;

    /** The name of the adapter associated with the connection. */
    public final String adapterName;

    /** The connection ID. */
    public final String connectionId;

    /**
     * Constructs a ConnectionInfo that wraps the provided underlying ConnectionInfo.
     *
     * @param underlying the underlying ConnectionInfo to wrap
     */
    protected ConnectionInfo(ConnectionInfo underlying) {
        assert underlying != null;
        this.underlying = underlying;
        incoming = underlying.incoming;
        adapterName = underlying.adapterName;
        connectionId = underlying.connectionId;
    }

    /**
     * Constructs a ConnectionInfo with the specified connection parameters.
     *
     * @param incoming whether this connection is incoming
     * @param adapterName the name of the adapter associated with this connection
     * @param connectionId the ID of this connection
     */
    protected ConnectionInfo(boolean incoming, String adapterName, String connectionId) {
        underlying = null;
        this.incoming = incoming;
        this.adapterName = adapterName;
        this.connectionId = connectionId;
    }
}
