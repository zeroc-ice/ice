//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised if an out-of-bounds condition occurs during unmarshaling.
 **/
public class UnmarshalOutOfBoundsException extends MarshalException
{
    public UnmarshalOutOfBoundsException()
    {
        super();
    }

    public UnmarshalOutOfBoundsException(Throwable cause)
    {
        super(cause);
    }

    public UnmarshalOutOfBoundsException(String reason)
    {
        super(reason);
    }

    public UnmarshalOutOfBoundsException(String reason, Throwable cause)
    {
        super(reason, cause);
    }

    public String ice_id()
    {
        return "::Ice::UnmarshalOutOfBoundsException";
    }

    /** @hidden */
    public static final long serialVersionUID = -1750105256021843030L;
}
