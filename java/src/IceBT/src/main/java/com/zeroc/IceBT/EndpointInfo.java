//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceBT;

/**
 * Provides access to Bluetooth endpoint information.
 **/
public abstract class EndpointInfo extends com.zeroc.Ice.EndpointInfo
{
    public EndpointInfo()
    {
        super();
        this.addr = "";
        this.uuid = "";
    }

    public EndpointInfo(com.zeroc.Ice.EndpointInfo underlying, int timeout, boolean compress, String addr, String uuid)
    {
        super(underlying, timeout, compress);
        this.addr = addr;
        this.uuid = uuid;
    }

    /**
     * The address configured with the endpoint.
     **/
    public String addr;

    /**
     * The UUID configured with the endpoint.
     **/
    public String uuid;

    public EndpointInfo clone()
    {
        return (EndpointInfo)super.clone();
    }

    /** @hidden */
    public static final long serialVersionUID = 4825481358879091449L;
}
