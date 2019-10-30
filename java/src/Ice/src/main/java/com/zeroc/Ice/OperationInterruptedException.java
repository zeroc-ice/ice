//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception indicates a request was interrupted.
 **/
public class OperationInterruptedException extends LocalException
{
    public OperationInterruptedException()
    {
    }

    public OperationInterruptedException(Throwable cause)
    {
        super(cause);
    }

    public String ice_id()
    {
        return "::Ice::OperationInterruptedException";
    }

    /** @hidden */
    public static final long serialVersionUID = -1099536335133286580L;
}
