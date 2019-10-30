//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception indicates that the connection has been gracefully shut down by the
 * server. The operation call that caused this exception has not been
 * executed by the server. In most cases you will not get this
 * exception, because the client will automatically retry the
 * operation call in case the server shut down the connection. However,
 * if upon retry the server shuts down the connection again, and the
 * retry limit has been reached, then this exception is propagated to
 * the application code.
 **/
public class CloseConnectionException extends ProtocolException
{
    public CloseConnectionException()
    {
        super();
    }

    public CloseConnectionException(Throwable cause)
    {
        super(cause);
    }

    public CloseConnectionException(String reason)
    {
        super(reason);
    }

    public CloseConnectionException(String reason, Throwable cause)
    {
        super(reason, cause);
    }

    public String ice_id()
    {
        return "::Ice::CloseConnectionException";
    }

    /** @hidden */
    public static final long serialVersionUID = 4166975853591251903L;
}
