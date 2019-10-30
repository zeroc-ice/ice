//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised if there was an error while parsing a
 * stringified identity.
 **/
public class IdentityParseException extends LocalException
{
    public IdentityParseException()
    {
        this.str = "";
    }

    public IdentityParseException(Throwable cause)
    {
        super(cause);
        this.str = "";
    }

    public IdentityParseException(String str)
    {
        this.str = str;
    }

    public IdentityParseException(String str, Throwable cause)
    {
        super(cause);
        this.str = str;
    }

    public String ice_id()
    {
        return "::Ice::IdentityParseException";
    }

    /**
     * Describes the failure and includes the string that could not be parsed.
     **/
    public String str;

    /** @hidden */
    public static final long serialVersionUID = 8547577763521735682L;
}
