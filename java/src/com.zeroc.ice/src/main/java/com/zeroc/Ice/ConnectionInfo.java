// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Base class providing access to the connection details. */
public class ConnectionInfo {
    /** The information of the underlying transport or null if there's no underlying transport. */
    public final ConnectionInfo underlying;

    /** Whether or not the connection is an incoming or outgoing connection. */
    public final boolean incoming;

    /** The name of the adapter associated with the connection. */
    public final String adapterName;

    /** The connection id. */
    public final String connectionId;

    /**
     * Creates a new ConnectionInfo that wraps the specified underlying connection info.
     *
     * @param underlying the underlying connection info to wrap
     */
    protected ConnectionInfo(ConnectionInfo underlying) {
        assert underlying != null;
        this.underlying = underlying;
        incoming = underlying.incoming;
        adapterName = underlying.adapterName;
        connectionId = underlying.connectionId;
    }

    /**
     * Creates a new ConnectionInfo with the specified connection parameters.
     *
     * @param incoming   whether this connection is incoming
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
