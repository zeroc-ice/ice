//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised if an operation call on a server raises a
 * local exception. Because local exceptions are not transmitted by
 * the Ice protocol, the client receives all local exceptions raised
 * by the server as {@link UnknownLocalException}. The only exception to this
 * rule are all exceptions derived from {@link RequestFailedException},
 * which are transmitted by the Ice protocol even though they are
 * declared <code>local</code>.
 **/
public class UnknownLocalException extends UnknownException
{
    public UnknownLocalException()
    {
        super();
    }

    public UnknownLocalException(Throwable cause)
    {
        super(cause);
    }

    public UnknownLocalException(String unknown)
    {
        super(unknown);
    }

    public UnknownLocalException(String unknown, Throwable cause)
    {
        super(unknown, cause);
    }

    public String ice_id()
    {
        return "::Ice::UnknownLocalException";
    }

    /** @hidden */
    public static final long serialVersionUID = 1374449481624773050L;
}
