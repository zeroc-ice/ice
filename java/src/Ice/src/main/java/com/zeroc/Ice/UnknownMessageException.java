//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception indicates that an unknown protocol message has been received.
 **/
public class UnknownMessageException extends ProtocolException
{
    public UnknownMessageException()
    {
        super();
    }

    public UnknownMessageException(Throwable cause)
    {
        super(cause);
    }

    public UnknownMessageException(String reason)
    {
        super(reason);
    }

    public UnknownMessageException(String reason, Throwable cause)
    {
        super(reason, cause);
    }

    public String ice_id()
    {
        return "::Ice::UnknownMessageException";
    }

    /** @hidden */
    public static final long serialVersionUID = 1625154579332341724L;
}
