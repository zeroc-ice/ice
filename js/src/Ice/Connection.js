//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { defineStruct } from "./Struct.js";
import { defineEnum } from "./EnumBase.js";
import { defineDictionary } from "./HashMap.js";
import { StreamHelpers } from "./StreamHelpers.js";

/**
 *  The batch compression option when flushing queued batch requests.
 **/
export const CompressBatch = defineEnum([['Yes', 0], ['No', 1], ['BasedOnProxy', 2]]);

/**
 *  Base class providing access to the connection details.
 **/
export class ConnectionInfo
{
    constructor(underlying = null, incoming = false, adapterName = "", connectionId = "")
    {
        this.underlying = underlying;
        this.incoming = incoming;
        this.adapterName = adapterName;
        this.connectionId = connectionId;
    }
}

/**
 *  Specifies the close semantics for Active Connection Management.
 **/
export const ACMClose = defineEnum([
    ['CloseOff', 0],
    ['CloseOnIdle', 1],
    ['CloseOnInvocation', 2],
    ['CloseOnInvocationAndIdle', 3],
    ['CloseOnIdleForceful', 4]]);

/**
 *  Specifies the heartbeat semantics for Active Connection Management.
 **/
export const ACMHeartbeat = defineEnum([
    ['HeartbeatOff', 0],
    ['HeartbeatOnDispatch', 1],
    ['HeartbeatOnIdle', 2],
    ['HeartbeatAlways', 3]]);

/**
 *  A collection of Active Connection Management configuration settings.
 **/
export class ACM
{
    constructor(timeout = 0, close = ACMClose.CloseOff, heartbeat = ACMHeartbeat.HeartbeatOff)
    {
        this.timeout = timeout;
        this.close = close;
        this.heartbeat = heartbeat;
    }
}

defineStruct(ACM, true, true);

/**
 *  Determines the behavior when manually closing a connection.
 **/
export const ConnectionClose = defineEnum([['Forcefully', 0], ['Gracefully', 1], ['GracefullyWithWait', 2]]);

/**
 *  Provides access to the connection details of an IP connection
 **/
export class IPConnectionInfo extends ConnectionInfo
{
    constructor(underlying, incoming, adapterName, connectionId, localAddress = "", localPort = -1, remoteAddress = "", remotePort = -1)
    {
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
export class TCPConnectionInfo extends IPConnectionInfo
{
    constructor(underlying, incoming, adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort, rcvSize = 0, sndSize = 0)
    {
        super(underlying, incoming, adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort);
        this.rcvSize = rcvSize;
        this.sndSize = sndSize;
    }
}

export const [HeaderDict, HeaderDictHelper] = defineDictionary(StreamHelpers.StringHelper, StreamHelpers.StringHelper, false);

/**
 *  Provides access to the connection details of a WebSocket connection
 **/
export class WSConnectionInfo extends ConnectionInfo
{
    constructor(underlying, incoming, adapterName, connectionId, headers = null)
    {
        super(underlying, incoming, adapterName, connectionId);
        this.headers = headers;
    }
}
