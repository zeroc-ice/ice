//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * The operation can only be invoked with a twoway request.
 *
 * This exception is raised if an attempt is made to invoke an
 * operation with <code>ice_oneway</code>, <code>ice_batchOneway</code>, <code>ice_datagram</code>,
 * or <code>ice_batchDatagram</code> and the operation has a return value,
 * out-parameters, or an exception specification.
 **/
public class TwowayOnlyException extends LocalException
{
    public TwowayOnlyException()
    {
        this.operation = "";
    }

    public TwowayOnlyException(Throwable cause)
    {
        super(cause);
        this.operation = "";
    }

    public TwowayOnlyException(String operation)
    {
        this.operation = operation;
    }

    public TwowayOnlyException(String operation, Throwable cause)
    {
        super(cause);
        this.operation = operation;
    }

    public String ice_id()
    {
        return "::Ice::TwowayOnlyException";
    }

    /**
     * The name of the operation that was invoked.
     **/
    public String operation;

    /** @hidden */
    public static final long serialVersionUID = -7036652448391478186L;
}
