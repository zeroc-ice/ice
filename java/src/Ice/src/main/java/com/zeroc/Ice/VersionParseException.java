//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised if there was an error while parsing a
 * version.
 **/
public class VersionParseException extends LocalException
{
    public VersionParseException()
    {
        this.str = "";
    }

    public VersionParseException(Throwable cause)
    {
        super(cause);
        this.str = "";
    }

    public VersionParseException(String str)
    {
        this.str = str;
    }

    public VersionParseException(String str, Throwable cause)
    {
        super(cause);
        this.str = str;
    }

    public String ice_id()
    {
        return "::Ice::VersionParseException";
    }

    /**
     * Describes the failure and includes the string that could not be parsed.
     **/
    public String str;

    /** @hidden */
    public static final long serialVersionUID = 8839069742920598766L;
}
