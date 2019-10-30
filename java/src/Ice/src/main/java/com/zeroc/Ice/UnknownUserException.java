//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * An operation raised an incorrect user exception.
 *
 * This exception is raised if an operation raises a
 * user exception that is not declared in the exception's
 * <code>throws</code> clause. Such undeclared exceptions are
 * not transmitted from the server to the client by the Ice
 * protocol, but instead the client just gets an
 * {@link UnknownUserException}. This is necessary in order to not violate
 * the contract established by an operation's signature: Only local
 * exceptions and user exceptions declared in the
 * <code>throws</code> clause can be raised.
 **/
public class UnknownUserException extends UnknownException
{
    public UnknownUserException()
    {
        super();
    }

    public UnknownUserException(Throwable cause)
    {
        super(cause);
    }

    public UnknownUserException(String unknown)
    {
        super(unknown);
    }

    public UnknownUserException(String unknown, Throwable cause)
    {
        super(unknown, cause);
    }

    public String ice_id()
    {
        return "::Ice::UnknownUserException";
    }

    /** @hidden */
    public static final long serialVersionUID = -6046568406824082586L;
}
