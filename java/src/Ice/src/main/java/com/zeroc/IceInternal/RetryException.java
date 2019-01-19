//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public class RetryException extends Exception
{
    public
    RetryException(com.zeroc.Ice.LocalException ex)
    {
        _ex = ex;
    }

    public com.zeroc.Ice.LocalException
    get()
    {
        return _ex;
    }

    private com.zeroc.Ice.LocalException _ex;

    public static final long serialVersionUID = 0L;
}
