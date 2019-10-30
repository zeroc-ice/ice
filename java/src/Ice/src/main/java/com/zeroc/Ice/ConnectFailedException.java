//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception indicates connection failures.
 **/
public class ConnectFailedException extends SocketException
{
    public ConnectFailedException()
    {
        super();
    }

    public ConnectFailedException(Throwable cause)
    {
        super(cause);
    }

    public ConnectFailedException(int error)
    {
        super(error);
    }

    public ConnectFailedException(int error, Throwable cause)
    {
        super(error, cause);
    }

    public String ice_id()
    {
        return "::Ice::ConnectFailedException";
    }

    /** @hidden */
    public static final long serialVersionUID = 3367444083227777090L;
}
