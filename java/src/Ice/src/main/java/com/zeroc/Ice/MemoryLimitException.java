//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised when Ice receives a request or reply
 * message whose size exceeds the limit specified by the
 * <code>Ice.MessageSizeMax</code> property.
 **/
public class MemoryLimitException extends MarshalException
{
    public MemoryLimitException()
    {
        super();
    }

    public MemoryLimitException(Throwable cause)
    {
        super(cause);
    }

    public MemoryLimitException(String reason)
    {
        super(reason);
    }

    public MemoryLimitException(String reason, Throwable cause)
    {
        super(reason, cause);
    }

    public String ice_id()
    {
        return "::Ice::MemoryLimitException";
    }

    /** @hidden */
    public static final long serialVersionUID = 8256664949263546187L;
}
