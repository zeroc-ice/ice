//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * Provides access to the details of an opaque endpoint.
 *
 * @see Endpoint
 **/
public abstract class OpaqueEndpointInfo extends EndpointInfo
{
    public OpaqueEndpointInfo()
    {
        super();
        this.rawEncoding = new EncodingVersion();
    }

    public OpaqueEndpointInfo(EndpointInfo underlying, int timeout, boolean compress, EncodingVersion rawEncoding, byte[] rawBytes)
    {
        super(underlying, timeout, compress);
        this.rawEncoding = rawEncoding;
        this.rawBytes = rawBytes;
    }

    /**
     * The encoding version of the opaque endpoint (to decode or
     * encode the rawBytes).
     **/
    public EncodingVersion rawEncoding;

    /**
     * The raw encoding of the opaque endpoint.
     **/
    public byte[] rawBytes;

    public OpaqueEndpointInfo clone()
    {
        return (OpaqueEndpointInfo)super.clone();
    }

    /** @hidden */
    public static final long serialVersionUID = 2439699764526521524L;
}
