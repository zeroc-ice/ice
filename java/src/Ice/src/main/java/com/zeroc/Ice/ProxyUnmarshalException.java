//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised if inconsistent data is received while unmarshaling a proxy.
 **/
public class ProxyUnmarshalException extends MarshalException
{
    public ProxyUnmarshalException()
    {
        super();
    }

    public ProxyUnmarshalException(Throwable cause)
    {
        super(cause);
    }

    public ProxyUnmarshalException(String reason)
    {
        super(reason);
    }

    public ProxyUnmarshalException(String reason, Throwable cause)
    {
        super(reason, cause);
    }

    public String ice_id()
    {
        return "::Ice::ProxyUnmarshalException";
    }

    /** @hidden */
    public static final long serialVersionUID = 2987361361391436704L;
}
