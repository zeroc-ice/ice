//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception indicates a connection failure for which
 * the server host actively refuses a connection.
 **/
public class ConnectionRefusedException extends ConnectFailedException
{
    public ConnectionRefusedException()
    {
        super();
    }

    public ConnectionRefusedException(Throwable cause)
    {
        super(cause);
    }

    public ConnectionRefusedException(int error)
    {
        super(error);
    }

    public ConnectionRefusedException(int error, Throwable cause)
    {
        super(error, cause);
    }

    public String ice_id()
    {
        return "::Ice::ConnectionRefusedException";
    }

    /** @hidden */
    public static final long serialVersionUID = 2368977428979563391L;
}
