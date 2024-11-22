// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * Provides access to the details of an opaque endpoint.
 *
 * @see Endpoint
 */
public abstract class OpaqueEndpointInfo extends EndpointInfo {
    /** The encoding version of the opaque endpoint (to decode or encode the rawBytes). */
    public EncodingVersion rawEncoding;

    /** The raw encoding of the opaque endpoint. */
    public byte[] rawBytes;
}
