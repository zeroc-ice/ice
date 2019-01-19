//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * Base class for all Ice system exceptions.
 * Ice system exceptions are currently Ice internal, non-documented
 * exceptions.
 **/
public abstract class SystemException extends Exception
{
    public SystemException()
    {
    }

    public SystemException(Throwable cause)
    {
        super(cause);
    }

    public static final long serialVersionUID = 0L;
}
