//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception indicates that an unknown reply status has been received.
 **/
public class UnknownReplyStatusException extends ProtocolException
{
    public UnknownReplyStatusException()
    {
        super();
    }

    public UnknownReplyStatusException(Throwable cause)
    {
        super(cause);
    }

    public UnknownReplyStatusException(String reason)
    {
        super(reason);
    }

    public UnknownReplyStatusException(String reason, Throwable cause)
    {
        super(reason, cause);
    }

    public String ice_id()
    {
        return "::Ice::UnknownReplyStatusException";
    }

    /** @hidden */
    public static final long serialVersionUID = -6816121886863135119L;
}
