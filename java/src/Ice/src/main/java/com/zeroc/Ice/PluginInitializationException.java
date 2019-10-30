//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception indicates that a failure occurred while initializing
 * a plug-in.
 **/
public class PluginInitializationException extends LocalException
{
    public PluginInitializationException()
    {
        this.reason = "";
    }

    public PluginInitializationException(Throwable cause)
    {
        super(cause);
        this.reason = "";
    }

    public PluginInitializationException(String reason)
    {
        this.reason = reason;
    }

    public PluginInitializationException(String reason, Throwable cause)
    {
        super(cause);
        this.reason = reason;
    }

    public String ice_id()
    {
        return "::Ice::PluginInitializationException";
    }

    /**
     * The reason for the failure.
     **/
    public String reason;

    /** @hidden */
    public static final long serialVersionUID = 1589933368626096169L;
}
