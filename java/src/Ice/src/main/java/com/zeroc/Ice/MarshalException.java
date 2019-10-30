//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception is raised for errors during marshaling or unmarshaling data.
 **/
public class MarshalException extends ProtocolException
{
    public MarshalException()
    {
        super();
    }

    public MarshalException(Throwable cause)
    {
        super(cause);
    }

    public MarshalException(String reason)
    {
        super(reason);
    }

    public MarshalException(String reason, Throwable cause)
    {
        super(reason, cause);
    }

    public String ice_id()
    {
        return "::Ice::MarshalException";
    }

    /** @hidden */
    public static final long serialVersionUID = -1332260000897066889L;
}
