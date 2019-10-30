//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * Base class providing access to the endpoint details.
 **/
public abstract class EndpointInfo implements java.lang.Cloneable
{
    public EndpointInfo()
    {
    }

    public EndpointInfo(EndpointInfo underlying, int timeout, boolean compress)
    {
        this.underlying = underlying;
        this.timeout = timeout;
        this.compress = compress;
    }

    /**
     * The information of the underyling endpoint of null if there's
     * no underlying endpoint.
     **/
    public EndpointInfo underlying;

    /**
     * The timeout for the endpoint in milliseconds. 0 means
     * non-blocking, -1 means no timeout.
     **/
    public int timeout;

    /**
     * Specifies whether or not compression should be used if
     * available when using this endpoint.
     **/
    public boolean compress;

    /**
     * Returns the type of the endpoint.
     * @return The endpoint type.
     **/
    public abstract short type();

    /**
     * Returns true if this endpoint is a datagram endpoint.
     * @return True for a datagram endpoint.
     **/
    public abstract boolean datagram();

    /**
     * Returns true if this endpoint is a secure endpoint.
     * @return True for a secure endpoint.
     **/
    public abstract boolean secure();

    public EndpointInfo clone()
    {
        EndpointInfo c = null;
        try
        {
            c = (EndpointInfo)super.clone();
        }
        catch(CloneNotSupportedException ex)
        {
            assert false; // impossible
        }
        return c;
    }

    /** @hidden */
    public static final long serialVersionUID = -7228250335304066764L;
}
