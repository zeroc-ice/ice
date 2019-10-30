//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * Provides access to the connection details of a WebSocket connection
 **/
public class WSConnectionInfo extends ConnectionInfo
{
    public WSConnectionInfo()
    {
        super();
    }

    public WSConnectionInfo(ConnectionInfo underlying, boolean incoming, String adapterName, String connectionId, java.util.Map<java.lang.String, java.lang.String> headers)
    {
        super(underlying, incoming, adapterName, connectionId);
        this.headers = headers;
    }

    /**
     * The headers from the HTTP upgrade request.
     **/
    public java.util.Map<java.lang.String, java.lang.String> headers;

    public WSConnectionInfo clone()
    {
        return (WSConnectionInfo)super.clone();
    }

    /** @hidden */
    public static final long serialVersionUID = 9085761366886580254L;
}
