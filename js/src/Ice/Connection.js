//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { defineDictionary } from "./HashMap.js";
import { StreamHelpers } from "./StreamHelpers.js";

/**
 *  Base class providing access to the connection details.
 **/
export class ConnectionInfo {
    constructor() {
        if (arguments.length === 3) {
            // ConnectionInfo(incoming, adapterName, connectionId)
            this._underlying = null;
            this._incoming = arguments[0];
            this._adapterName = arguments[1];
            this._connectionId = arguments[2];
        } else {
            // ConnectionInfo(underlying)
            DEV: console.assert(arguments.length === 1);
            DEV: console.assert(arguments[0] instanceof ConnectionInfo);
            this._underlying = arguments[0];
            this._incoming = this._underlying.incoming;
            this._adapterName = this._underlying.adapterName;
            this._connectionId = this._underlying.connectionId;
        }
    }

    get underlying() {
        return this._underlying;
    }

    get incoming() {
        return this._incoming;
    }

    get adapterName() {
        return this._adapterName;
    }

    get connectionId() {
        return this._connectionId;
    }
}

/**
 *  Provides access to the connection details of an IP connection
 **/
export class IPConnectionInfo extends ConnectionInfo {
    constructor(
        incoming,
        adapterName,
        connectionId,
        localAddress = "",
        localPort = -1,
        remoteAddress = "",
        remotePort = -1,
    ) {
        super(incoming, adapterName, connectionId);
        this._localAddress = localAddress;
        this._localPort = localPort;
        this._remoteAddress = remoteAddress;
        this._remotePort = remotePort;
    }

    get localAddress() {
        return this._localAddress;
    }

    get localPort() {
        return this._localPort;
    }

    get remoteAddress() {
        return this._remoteAddress;
    }

    get remotePort() {
        return this._remotePort;
    }
}

/**
 *  Provides access to the connection details of a TCP connection
 **/
export class TCPConnectionInfo extends IPConnectionInfo {
    constructor(
        incoming,
        adapterName,
        connectionId,
        localAddress = "",
        localPort = -1,
        remoteAddress = "",
        remotePort = -1,
        rcvSize = 0,
        sndSize = 0,
    ) {
        super(incoming, adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort);
        this._rcvSize = rcvSize;
        this._sndSize = sndSize;
    }

    get rcvSize() {
        return this._rcvSize;
    }

    get sndSize() {
        return this._sndSize;
    }
}

export const [HeaderDict, HeaderDictHelper] = defineDictionary(
    StreamHelpers.StringHelper,
    StreamHelpers.StringHelper,
    false,
);

/**
 *  Provides access to the connection details of a WebSocket connection
 **/
export class WSConnectionInfo extends ConnectionInfo {
    constructor(underlying, headers) {
        super(underlying);
        this._headers = headers;
    }

    get headers() {
        return this._headers;
    }
}
