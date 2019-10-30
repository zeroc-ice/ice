//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * Base class providing access to the connection details.
 **/
public class ConnectionInfo implements java.lang.Cloneable
{
    public ConnectionInfo()
    {
        this.adapterName = "";
        this.connectionId = "";
    }

    public ConnectionInfo(ConnectionInfo underlying, boolean incoming, String adapterName, String connectionId)
    {
        this.underlying = underlying;
        this.incoming = incoming;
        this.adapterName = adapterName;
        this.connectionId = connectionId;
    }

    /**
     * The information of the underyling transport or null if there's
     * no underlying transport.
     **/
    public ConnectionInfo underlying;

    /**
     * Whether or not the connection is an incoming or outgoing
     * connection.
     **/
    public boolean incoming;

    /**
     * The name of the adapter associated with the connection.
     **/
    public String adapterName;

    /**
     * The connection id.
     **/
    public String connectionId;

    public ConnectionInfo clone()
    {
        ConnectionInfo c = null;
        try
        {
            c = (ConnectionInfo)super.clone();
        }
        catch(CloneNotSupportedException ex)
        {
            assert false; // impossible
        }
        return c;
    }

    /** @hidden */
    public static final long serialVersionUID = -8138441397879390442L;
}
