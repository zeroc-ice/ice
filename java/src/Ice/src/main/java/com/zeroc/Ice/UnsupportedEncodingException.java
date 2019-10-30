//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * This exception indicates an unsupported data encoding version.
 **/
public class UnsupportedEncodingException extends ProtocolException
{
    public UnsupportedEncodingException()
    {
        super();
        this.bad = new EncodingVersion();
        this.supported = new EncodingVersion();
    }

    public UnsupportedEncodingException(Throwable cause)
    {
        super(cause);
        this.bad = new EncodingVersion();
        this.supported = new EncodingVersion();
    }

    public UnsupportedEncodingException(String reason, EncodingVersion bad, EncodingVersion supported)
    {
        super(reason);
        this.bad = bad;
        this.supported = supported;
    }

    public UnsupportedEncodingException(String reason, EncodingVersion bad, EncodingVersion supported, Throwable cause)
    {
        super(reason, cause);
        this.bad = bad;
        this.supported = supported;
    }

    public String ice_id()
    {
        return "::Ice::UnsupportedEncodingException";
    }

    /**
     * The version of the unsupported encoding.
     **/
    public EncodingVersion bad;

    /**
     * The version of the encoding that is supported.
     **/
    public EncodingVersion supported;

    /** @hidden */
    public static final long serialVersionUID = -4128311673376608852L;
}
