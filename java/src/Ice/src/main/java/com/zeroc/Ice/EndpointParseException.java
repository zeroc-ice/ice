//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised if there was an error while parsing an
 * endpoint.
 **/
public class EndpointParseException extends LocalException
{
    public EndpointParseException()
    {
        this.str = "";
    }

    public EndpointParseException(Throwable cause)
    {
        super(cause);
        this.str = "";
    }

    public EndpointParseException(String str)
    {
        this.str = str;
    }

    public EndpointParseException(String str, Throwable cause)
    {
        super(cause);
        this.str = str;
    }

    public String ice_id()
    {
        return "::Ice::EndpointParseException";
    }

    /**
     * Describes the failure and includes the string that could not be parsed.
     **/
    public String str;

    /** @hidden */
    public static final long serialVersionUID = 2726204311748106167L;
}
