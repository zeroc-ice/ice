//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * A datagram exceeds the configured size.
 *
 * This exception is raised if a datagram exceeds the configured send or receive buffer
 * size, or exceeds the maximum payload size of a UDP packet (65507 bytes).
 **/
public class DatagramLimitException extends ProtocolException
{
    public DatagramLimitException()
    {
        super();
    }

    public DatagramLimitException(Throwable cause)
    {
        super(cause);
    }

    public DatagramLimitException(String reason)
    {
        super(reason);
    }

    public DatagramLimitException(String reason, Throwable cause)
    {
        super(reason, cause);
    }

    public String ice_id()
    {
        return "::Ice::DatagramLimitException";
    }

    /** @hidden */
    public static final long serialVersionUID = -783492847222783613L;
}
