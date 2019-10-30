//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception indicates that an asynchronous invocation failed
 * because it was canceled explicitly by the user.
 **/
public class InvocationCanceledException extends LocalException
{
    public InvocationCanceledException()
    {
    }

    public InvocationCanceledException(Throwable cause)
    {
        super(cause);
    }

    public String ice_id()
    {
        return "::Ice::InvocationCanceledException";
    }

    /** @hidden */
    public static final long serialVersionUID = -6429704142271073768L;
}
