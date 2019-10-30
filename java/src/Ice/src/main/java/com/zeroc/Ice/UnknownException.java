//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised if an operation call on a server raises an
 * unknown exception. For example, for C++, this exception is raised
 * if the server throws a C++ exception that is not directly or
 * indirectly derived from <code>Ice::LocalException</code> or
 * <code>Ice::UserException</code>.
 **/
public class UnknownException extends LocalException
{
    public UnknownException()
    {
        this.unknown = "";
    }

    public UnknownException(Throwable cause)
    {
        super(cause);
        this.unknown = "";
    }

    public UnknownException(String unknown)
    {
        this.unknown = unknown;
    }

    public UnknownException(String unknown, Throwable cause)
    {
        super(cause);
        this.unknown = unknown;
    }

    public String ice_id()
    {
        return "::Ice::UnknownException";
    }

    /**
     * This field is set to the textual representation of the unknown
     * exception if available.
     **/
    public String unknown;

    /** @hidden */
    public static final long serialVersionUID = 4845487294380422868L;
}
