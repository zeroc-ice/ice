//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceSSL;

/**
 * Provides access to an SSL endpoint information.
 **/
public abstract class EndpointInfo extends com.zeroc.Ice.EndpointInfo
{
    public EndpointInfo()
    {
        super();
    }

    public EndpointInfo(com.zeroc.Ice.EndpointInfo underlying, int timeout, boolean compress)
    {
        super(underlying, timeout, compress);
    }

    public EndpointInfo clone()
    {
        return (EndpointInfo)super.clone();
    }

    /** @hidden */
    public static final long serialVersionUID = 5490788644436785361L;
}
