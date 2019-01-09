// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.Ice;

/**
 * Base class for all Ice run-time exceptions.
 **/
public abstract class LocalException extends Exception
{
    public LocalException()
    {
    }

    public LocalException(Throwable cause)
    {
        super(cause);
    }

    public static final long serialVersionUID = 0L;
}
