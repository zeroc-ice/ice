//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception indicates that a message size is less
 * than the minimum required size.
 **/
public class IllegalMessageSizeException extends ProtocolException
{
    public IllegalMessageSizeException()
    {
        super();
    }

    public IllegalMessageSizeException(Throwable cause)
    {
        super(cause);
    }

    public IllegalMessageSizeException(String reason)
    {
        super(reason);
    }

    public IllegalMessageSizeException(String reason, Throwable cause)
    {
        super(reason, cause);
    }

    public String ice_id()
    {
        return "::Ice::IllegalMessageSizeException";
    }

    /** @hidden */
    public static final long serialVersionUID = 3581741698610780247L;
}
