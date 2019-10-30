//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception indicates a problem with compressing or uncompressing data.
 **/
public class CompressionException extends ProtocolException
{
    public CompressionException()
    {
        super();
    }

    public CompressionException(Throwable cause)
    {
        super(cause);
    }

    public CompressionException(String reason)
    {
        super(reason);
    }

    public CompressionException(String reason, Throwable cause)
    {
        super(reason, cause);
    }

    public String ice_id()
    {
        return "::Ice::CompressionException";
    }

    /** @hidden */
    public static final long serialVersionUID = -3980762816174249071L;
}
