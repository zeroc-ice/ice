//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { Ice as Ice_Version } from "./Version.js";
const { EncodingVersion } = Ice_Version;

/**
 *  Base class providing access to the endpoint details.
 **/
export class EndpointInfo
{
    constructor(underlying = null, timeout = 0, compress = false)
    {
        this.underlying = underlying;
        this.timeout = timeout;
        this.compress = compress;
    }
}

/**
 *  Provides access to the address details of a IP endpoint.
 *  @see Endpoint
 **/
export class IPEndpointInfo extends EndpointInfo
{
    constructor(underlying, timeout, compress, host = "", port = 0, sourceAddress = "")
    {
        super(underlying, timeout, compress);
        this.host = host;
        this.port = port;
        this.sourceAddress = sourceAddress;
    }
}

/**
 *  Provides access to a TCP endpoint information.
 *  @see Endpoint
 **/
export class TCPEndpointInfo extends IPEndpointInfo
{
    constructor(underlying, timeout, compress, host, port, sourceAddress)
    {
        super(underlying, timeout, compress, host, port, sourceAddress);
    }
}

/**
 *  Provides access to a WebSocket endpoint information.
 **/
export class WSEndpointInfo extends EndpointInfo
{
    constructor(underlying, timeout, compress, resource = "")
    {
        super(underlying, timeout, compress);
        this.resource = resource;
    }
};

/**
 *  Provides access to the details of an opaque endpoint.
 *  @see Endpoint
 **/
export class OpaqueEndpointInfo extends EndpointInfo
{
    constructor(underlying, timeout, compress, rawEncoding = new EncodingVersion(), rawBytes = null)
    {
        super(underlying, timeout, compress);
        this.rawEncoding = rawEncoding;
        this.rawBytes = rawBytes;
    }
}
