//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception indicates a connection closure timeout condition.
 **/
public class CloseTimeoutException extends TimeoutException
{
    public CloseTimeoutException()
    {
        super();
    }

    public CloseTimeoutException(Throwable cause)
    {
        super(cause);
    }

    public String ice_id()
    {
        return "::Ice::CloseTimeoutException";
    }

    /** @hidden */
    public static final long serialVersionUID = -702193953324375086L;
}
