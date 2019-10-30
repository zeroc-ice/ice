//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * Provides access to a WebSocket endpoint information.
 **/
public abstract class WSEndpointInfo extends EndpointInfo
{
    public WSEndpointInfo()
    {
        super();
        this.resource = "";
    }

    public WSEndpointInfo(EndpointInfo underlying, int timeout, boolean compress, String resource)
    {
        super(underlying, timeout, compress);
        this.resource = resource;
    }

    /**
     * The URI configured with the endpoint.
     **/
    public String resource;

    public WSEndpointInfo clone()
    {
        return (WSEndpointInfo)super.clone();
    }

    /** @hidden */
    public static final long serialVersionUID = 6685502615474659064L;
}
