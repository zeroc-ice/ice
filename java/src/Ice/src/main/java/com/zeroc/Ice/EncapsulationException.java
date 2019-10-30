//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception indicates a malformed data encapsulation.
 **/
public class EncapsulationException extends MarshalException
{
    public EncapsulationException()
    {
        super();
    }

    public EncapsulationException(Throwable cause)
    {
        super(cause);
    }

    public EncapsulationException(String reason)
    {
        super(reason);
    }

    public EncapsulationException(String reason, Throwable cause)
    {
        super(reason, cause);
    }

    public String ice_id()
    {
        return "::Ice::EncapsulationException";
    }

    /** @hidden */
    public static final long serialVersionUID = 6924075986153135793L;
}
