// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Base class providing access to the connection details. */
public class ConnectionInfo implements Cloneable {
    /** The information of the underyling transport or null if there's no underlying transport. */
    public ConnectionInfo underlying;

    /** Whether or not the connection is an incoming or outgoing connection. */
    public boolean incoming;

    /** The name of the adapter associated with the connection. */
    public String adapterName = "";

    /** The connection id. */
    public String connectionId = "";

    @Override
    public ConnectionInfo clone() {
        try {
            return (ConnectionInfo) super.clone();
        } catch (CloneNotSupportedException ex) {
            assert false; // impossible
            return null;
        }
    }
}
