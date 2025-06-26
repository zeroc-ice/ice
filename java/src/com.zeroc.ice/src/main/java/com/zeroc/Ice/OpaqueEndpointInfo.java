// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * Provides access to the details of an opaque endpoint.
 *
 * @see Endpoint
 */
public final class OpaqueEndpointInfo extends EndpointInfo {
    /** The encoding version of the opaque endpoint (to decode or encode the rawBytes). */
    public final EncodingVersion rawEncoding;

    /** The raw encoding of the opaque endpoint. */
    public final byte[] rawBytes;

    private final short _type;

    @Override
    public short type() {
        return _type;
    }

    // internal constructor
    OpaqueEndpointInfo(short type, EncodingVersion rawEncoding, byte[] rawBytes) {
        super(false);
        this._type = type;
        this.rawEncoding = rawEncoding;
        this.rawBytes = rawBytes;
    }
}
