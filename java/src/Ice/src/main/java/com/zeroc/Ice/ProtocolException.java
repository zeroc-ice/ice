//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * A generic exception base for all kinds of protocol error
 * conditions.
 **/
public class ProtocolException extends LocalException
{
    public ProtocolException()
    {
        this.reason = "";
    }

    public ProtocolException(Throwable cause)
    {
        super(cause);
        this.reason = "";
    }

    public ProtocolException(String reason)
    {
        this.reason = reason;
    }

    public ProtocolException(String reason, Throwable cause)
    {
        super(cause);
        this.reason = reason;
    }

    public String ice_id()
    {
        return "::Ice::ProtocolException";
    }

    /**
     * The reason for the failure.
     **/
    public String reason;

    /** @hidden */
    public static final long serialVersionUID = 6046324714700663302L;
}
