//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception indicates that an invocation failed because it timed
 * out.
 **/
public class InvocationTimeoutException extends TimeoutException
{
    public InvocationTimeoutException()
    {
        super();
    }

    public InvocationTimeoutException(Throwable cause)
    {
        super(cause);
    }

    public String ice_id()
    {
        return "::Ice::InvocationTimeoutException";
    }

    /** @hidden */
    public static final long serialVersionUID = -4956443780705036860L;
}
