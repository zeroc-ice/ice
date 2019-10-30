//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception indicates a DNS problem. For details on the cause,
 * {@link DNSException#error} should be inspected.
 **/
public class DNSException extends LocalException
{
    public DNSException()
    {
        this.error = 0;
        this.host = "";
    }

    public DNSException(Throwable cause)
    {
        super(cause);
        this.error = 0;
        this.host = "";
    }

    public DNSException(int error, String host)
    {
        this.error = error;
        this.host = host;
    }

    public DNSException(int error, String host, Throwable cause)
    {
        super(cause);
        this.error = error;
        this.host = host;
    }

    public String ice_id()
    {
        return "::Ice::DNSException";
    }

    /**
     * The error number describing the DNS problem. For C++ and Unix,
     * this is equivalent to <code>h_errno</code>. For C++ and
     * Windows, this is the value returned by
     * <code>WSAGetLastError()</code>.
     **/
    public int error;

    /**
     * The host name that could not be resolved.
     **/
    public String host;

    /** @hidden */
    public static final long serialVersionUID = 824453629913156786L;
}
