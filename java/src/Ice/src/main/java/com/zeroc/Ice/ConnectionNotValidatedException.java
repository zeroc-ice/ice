//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised if a message is received over a connection
 * that is not yet validated.
 **/
public class ConnectionNotValidatedException extends ProtocolException
{
    public ConnectionNotValidatedException()
    {
        super();
    }

    public ConnectionNotValidatedException(Throwable cause)
    {
        super(cause);
    }

    public ConnectionNotValidatedException(String reason)
    {
        super(reason);
    }

    public ConnectionNotValidatedException(String reason, Throwable cause)
    {
        super(reason, cause);
    }

    public String ice_id()
    {
        return "::Ice::ConnectionNotValidatedException";
    }

    /** @hidden */
    public static final long serialVersionUID = 1338347369471941150L;
}
