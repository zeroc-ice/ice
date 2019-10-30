//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception indicates an unsupported protocol version.
 **/
public class UnsupportedProtocolException extends ProtocolException
{
    public UnsupportedProtocolException()
    {
        super();
        this.bad = new ProtocolVersion();
        this.supported = new ProtocolVersion();
    }

    public UnsupportedProtocolException(Throwable cause)
    {
        super(cause);
        this.bad = new ProtocolVersion();
        this.supported = new ProtocolVersion();
    }

    public UnsupportedProtocolException(String reason, ProtocolVersion bad, ProtocolVersion supported)
    {
        super(reason);
        this.bad = bad;
        this.supported = supported;
    }

    public UnsupportedProtocolException(String reason, ProtocolVersion bad, ProtocolVersion supported, Throwable cause)
    {
        super(reason, cause);
        this.bad = bad;
        this.supported = supported;
    }

    public String ice_id()
    {
        return "::Ice::UnsupportedProtocolException";
    }

    /**
     * The version of the unsupported protocol.
     **/
    public ProtocolVersion bad;

    /**
     * The version of the protocol that is supported.
     **/
    public ProtocolVersion supported;

    /** @hidden */
    public static final long serialVersionUID = 811091383730564025L;
}
