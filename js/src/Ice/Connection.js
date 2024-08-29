//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { defineDictionary } from "./HashMap.js";
import { StreamHelpers } from "./StreamHelpers.js";

/**
 *  Base class providing access to the connection details.
 **/
export class ConnectionInfo {
    constructor(underlying = null, incoming = false, adapterName = "", connectionId = "") {
        this.underlying = underlying;
        this.incoming = incoming;
        this.adapterName = adapterName;
        this.connectionId = connectionId;
    }
}

/**
 *  Provides access to the connection details of an IP connection
 **/
export class IPConnectionInfo extends ConnectionInfo {
    constructor(
        underlying,
        incoming,
        adapterName,
        connectionId,
        localAddress = "",
        localPort = -1,
        remoteAddress = "",
        remotePort = -1,
    ) {
        super(underlying, incoming, adapterName, connectionId);
        this.localAddress = localAddress;
        this.localPort = localPort;
        this.remoteAddress = remoteAddress;
        this.remotePort = remotePort;
    }
}

/**
 *  Provides access to the connection details of a TCP connection
 **/
export class TCPConnectionInfo extends IPConnectionInfo {
    constructor(
        underlying,
        incoming,
        adapterName,
        connectionId,
        localAddress,
        localPort,
        remoteAddress,
        remotePort,
        rcvSize = 0,
        sndSize = 0,
    ) {
        super(underlying, incoming, adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort);
        this.rcvSize = rcvSize;
        this.sndSize = sndSize;
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
    constructor(underlying, incoming, adapterName, connectionId, headers = null) {
        super(underlying, incoming, adapterName, connectionId);
        this.headers = headers;
    }
}
