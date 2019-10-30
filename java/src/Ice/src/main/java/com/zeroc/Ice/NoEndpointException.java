//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised if no suitable endpoint is available.
 **/
public class NoEndpointException extends LocalException
{
    public NoEndpointException()
    {
        this.proxy = "";
    }

    public NoEndpointException(Throwable cause)
    {
        super(cause);
        this.proxy = "";
    }

    public NoEndpointException(String proxy)
    {
        this.proxy = proxy;
    }

    public NoEndpointException(String proxy, Throwable cause)
    {
        super(cause);
        this.proxy = proxy;
    }

    public String ice_id()
    {
        return "::Ice::NoEndpointException";
    }

    /**
     * The stringified proxy for which no suitable endpoint is
     * available.
     **/
    public String proxy;

    /** @hidden */
    public static final long serialVersionUID = -5026638954785808518L;
}
