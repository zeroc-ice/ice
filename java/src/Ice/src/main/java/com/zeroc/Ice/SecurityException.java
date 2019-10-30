//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception indicates a failure in a security subsystem,
 * such as the IceSSL plug-in.
 **/
public class SecurityException extends LocalException
{
    public SecurityException()
    {
        this.reason = "";
    }

    public SecurityException(Throwable cause)
    {
        super(cause);
        this.reason = "";
    }

    public SecurityException(String reason)
    {
        this.reason = reason;
    }

    public SecurityException(String reason, Throwable cause)
    {
        super(cause);
        this.reason = reason;
    }

    public String ice_id()
    {
        return "::Ice::SecurityException";
    }

    /**
     * The reason for the failure.
     **/
    public String reason;

    /** @hidden */
    public static final long serialVersionUID = 7929245908983964710L;
}
