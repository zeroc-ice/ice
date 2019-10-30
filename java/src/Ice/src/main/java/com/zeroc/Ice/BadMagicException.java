//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception indicates that a message did not start with the expected
 * magic number ('I', 'c', 'e', 'P').
 **/
public class BadMagicException extends ProtocolException
{
    public BadMagicException()
    {
        super();
    }

    public BadMagicException(Throwable cause)
    {
        super(cause);
    }

    public BadMagicException(String reason, byte[] badMagic)
    {
        super(reason);
        this.badMagic = badMagic;
    }

    public BadMagicException(String reason, byte[] badMagic, Throwable cause)
    {
        super(reason, cause);
        this.badMagic = badMagic;
    }

    public String ice_id()
    {
        return "::Ice::BadMagicException";
    }

    /**
     * A sequence containing the first four bytes of the incorrect message.
     **/
    public byte[] badMagic;

    /** @hidden */
    public static final long serialVersionUID = -3934807911473944716L;
}
