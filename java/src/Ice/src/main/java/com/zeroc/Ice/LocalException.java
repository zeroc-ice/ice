// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
