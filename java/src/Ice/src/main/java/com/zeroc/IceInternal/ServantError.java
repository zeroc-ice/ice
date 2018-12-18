// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.IceInternal;

final public class ServantError extends java.lang.Error
{
    public ServantError(String reason)
    {
        super(reason);
    }

    public ServantError(String reason, Throwable cause)
    {
        super(reason, cause);
    }

    public ServantError(Throwable cause)
    {
        super(cause);
    }

    public static final long serialVersionUID = 0L;
}
