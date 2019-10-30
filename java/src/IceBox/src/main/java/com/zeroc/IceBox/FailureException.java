//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceBox;

/**
 * This exception is a general failure notification. It is thrown
 * for errors such as a service encountering an error during
 * initialization, or the service manager being unable
 * to load a service executable.
 **/
public class FailureException extends com.zeroc.Ice.LocalException
{
    public FailureException()
    {
        this.reason = "";
    }

    public FailureException(Throwable cause)
    {
        super(cause);
        this.reason = "";
    }

    public FailureException(String reason)
    {
        this.reason = reason;
    }

    public FailureException(String reason, Throwable cause)
    {
        super(cause);
        this.reason = reason;
    }

    public String ice_id()
    {
        return "::IceBox::FailureException";
    }

    /**
     * The reason for the failure.
     **/
    public String reason;

    /** @hidden */
    public static final long serialVersionUID = -7740030157337496059L;
}
