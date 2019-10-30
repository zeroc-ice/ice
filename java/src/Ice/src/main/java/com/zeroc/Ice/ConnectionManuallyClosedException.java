//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised by an operation call if the application
 * closes the connection locally using {@link Connection#close}.
 *
 * @see Connection#close
 **/
public class ConnectionManuallyClosedException extends LocalException
{
    public ConnectionManuallyClosedException()
    {
    }

    public ConnectionManuallyClosedException(Throwable cause)
    {
        super(cause);
    }

    public ConnectionManuallyClosedException(boolean graceful)
    {
        this.graceful = graceful;
    }

    public ConnectionManuallyClosedException(boolean graceful, Throwable cause)
    {
        super(cause);
        this.graceful = graceful;
    }

    public String ice_id()
    {
        return "::Ice::ConnectionManuallyClosedException";
    }

    /**
     * True if the connection was closed gracefully, false otherwise.
     **/
    public boolean graceful;

    /** @hidden */
    public static final long serialVersionUID = 1412128468475443570L;
}
