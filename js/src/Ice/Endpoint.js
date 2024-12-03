// Copyright (c) ZeroC, Inc.

import { InitializationException } from "./LocalExceptions.js";

/**
 *  Base class providing access to the endpoint details.
 **/
export class EndpointInfo {
    constructor() {
        if (arguments.length === 2) {
            // EndpointInfo(timeout, compress)
            this._underlying = null;
            this._timeout = arguments[0];
            this._compress = arguments[1];
        } else {
            // EndpointInfo(underlying)
            DEV: console.assert(arguments.length === 1);
            DEV: console.assert(arguments[0] instanceof EndpointInfo);
            this._underlying = arguments[0];
            this._timeout = arguments[0].timeout;
            this._compress = arguments[0].compress;
        }
    }

    get timeout() {
        return this._timeout;
    }

    get compress() {
        return this._compress;
    }

    get underlying() {
        return this._underlying;
    }

    type() {
        return this._underlying?.type() ?? -1;
    }

    datagram() {
        return this._underlying?.datagram() ?? false;
    }

    secure() {
        return this._underlying?.secure() ?? false;
    }
}

/**
 *  Provides access to the address details of a IP endpoint.
 *  @see {@link Endpoint}
 **/
export class IPEndpointInfo extends EndpointInfo {
    constructor(timeout, compress, host, port, sourceAddress, type) {
        super(timeout, compress);
        this._host = host;
        this._port = port;
        this._sourceAddr = sourceAddress;
        this._type = type;
    }

    get host() {
        return this._host;
    }

    get port() {
        return this._port;
    }

    get sourceAddress() {
        return this._sourceAddr;
    }
}

/**
 *  Provides access to a TCP endpoint information.
 *  @see {@link Endpoint}
 **/
export class TCPEndpointInfo extends IPEndpointInfo {
    constructor(timeout, compress, host, port, sourceAddress, type, secure) {
        super(timeout, compress, host, port, sourceAddress);
        this._type = type;
        this._secure = secure;
    }

    type() {
        return this._type;
    }

    secure() {
        return this._secure;
    }
}

/**
 *  Provides access to a WebSocket endpoint information.
 **/
export class WSEndpointInfo extends EndpointInfo {
    constructor(underlying, resource) {
        super(underlying);
        this._resource = resource;
    }

    get resource() {
        return this._resource;
    }
}

/**
 *  Provides access to the details of an opaque endpoint.
 *  @see {@link Endpoint}
 **/
export class OpaqueEndpointInfo extends EndpointInfo {
    constructor(type, rawEncoding, rawBytes) {
        super(-1, false);
        this._type = type;
        this._rawEncoding = rawEncoding;
        this._rawBytes = rawBytes;
    }

    type() {
        return this._type;
    }

    get rawEncoding() {
        return this._rawEncoding;
    }

    get rawBytes() {
        return this._rawBytes;
    }
}
