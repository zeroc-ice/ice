//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception indicates a connection establishment timeout condition.
 **/
public class ConnectTimeoutException extends TimeoutException
{
    public ConnectTimeoutException()
    {
        super();
    }

    public ConnectTimeoutException(Throwable cause)
    {
        super(cause);
    }

    public String ice_id()
    {
        return "::Ice::ConnectTimeoutException";
    }

    /** @hidden */
    public static final long serialVersionUID = -1271371420507272518L;
}
