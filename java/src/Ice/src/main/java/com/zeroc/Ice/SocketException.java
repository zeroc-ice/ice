//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception indicates socket errors.
 **/
public class SocketException extends SyscallException
{
    public SocketException()
    {
        super();
    }

    public SocketException(Throwable cause)
    {
        super(cause);
    }

    public SocketException(int error)
    {
        super(error);
    }

    public SocketException(int error, Throwable cause)
    {
        super(error, cause);
    }

    public String ice_id()
    {
        return "::Ice::SocketException";
    }

    /** @hidden */
    public static final long serialVersionUID = -7634050967564791782L;
}
