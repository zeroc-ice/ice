//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised if there was an error while parsing an
 * endpoint selection type.
 **/
public class EndpointSelectionTypeParseException extends LocalException
{
    public EndpointSelectionTypeParseException()
    {
        this.str = "";
    }

    public EndpointSelectionTypeParseException(Throwable cause)
    {
        super(cause);
        this.str = "";
    }

    public EndpointSelectionTypeParseException(String str)
    {
        this.str = str;
    }

    public EndpointSelectionTypeParseException(String str, Throwable cause)
    {
        super(cause);
        this.str = str;
    }

    public String ice_id()
    {
        return "::Ice::EndpointSelectionTypeParseException";
    }

    /**
     * Describes the failure and includes the string that could not be parsed.
     **/
    public String str;

    /** @hidden */
    public static final long serialVersionUID = 5767960053089269935L;
}
