//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception indicates that an attempt has been made to
 * change the connection properties of a fixed proxy.
 **/
public class FixedProxyException extends LocalException
{
    public FixedProxyException()
    {
    }

    public FixedProxyException(Throwable cause)
    {
        super(cause);
    }

    public String ice_id()
    {
        return "::Ice::FixedProxyException";
    }

    /** @hidden */
    public static final long serialVersionUID = 3198117120780643493L;
}
