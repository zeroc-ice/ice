//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised if there was an error while parsing a
 * stringified proxy.
 **/
public class ProxyParseException extends LocalException
{
    public ProxyParseException()
    {
        this.str = "";
    }

    public ProxyParseException(Throwable cause)
    {
        super(cause);
        this.str = "";
    }

    public ProxyParseException(String str)
    {
        this.str = str;
    }

    public ProxyParseException(String str, Throwable cause)
    {
        super(cause);
        this.str = str;
    }

    public String ice_id()
    {
        return "::Ice::ProxyParseException";
    }

    /**
     * Describes the failure and includes the string that could not be parsed.
     **/
    public String str;

    /** @hidden */
    public static final long serialVersionUID = 1426520612608626840L;
}
