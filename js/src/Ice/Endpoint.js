// Copyright (c) ZeroC, Inc.

/**
 *  Base class providing access to the endpoint details.
 **/
export class EndpointInfo {
    constructor(underlying, timeout, compress) {
        if (underlying === null) {
            // EndpointInfo(timeout, compress)
            this._underlying = null;
            this._timeout = timeout;
            this._compress = compress;
        } else {
            this._underlying = underlying;
            this._timeout = underlying.timeout;
            this._compress = underlying.compress;
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
    constructor(timeout, compress, host, port, sourceAddress) {
        super(null, timeout, compress);
        this._host = host;
        this._port = port;
        this._sourceAddr = sourceAddress;
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
        super(null, -1, false);
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
