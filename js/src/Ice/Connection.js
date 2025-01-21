// Copyright (c) ZeroC, Inc.

/**
 *  Base class providing access to the connection details.
 **/
export class ConnectionInfo {
    constructor(underlying, adapterName, connectionId) {
        if (underlying === null) {
            this._underlying = null;
            this._adapterName = adapterName;
            this._connectionId = connectionId;
        } else {
            this._underlying = underlying;
            this._adapterName = underlying.adapterName;
            this._connectionId = underlying.connectionId;
        }
    }

    get underlying() {
        return this._underlying;
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
    constructor(adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort) {
        super(null, adapterName, connectionId);
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
    constructor(adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort, sndSize) {
        super(adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort);
        this._sndSize = sndSize;
    }

    get sndSize() {
        return this._sndSize;
    }
}

/**
 *  Provides access to the connection details of a WebSocket connection
 **/
export class WSConnectionInfo extends ConnectionInfo {}
