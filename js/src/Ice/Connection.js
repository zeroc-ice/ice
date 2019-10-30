//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

/* eslint-disable */
/* jshint ignore: start */

/* slice2js browser-bundle-skip */
(function(module, require, exports)
{
/* slice2js browser-bundle-skip-end */
/* slice2js browser-bundle-skip */
    const _ModuleRegistry = require("../Ice/ModuleRegistry").Ice._ModuleRegistry;
    const Ice = _ModuleRegistry.require(module,
    [
        "../Ice/Object",
        "../Ice/Value",
        "../Ice/ObjectPrx",
        "../Ice/Struct",
        "../Ice/EnumBase",
        "../Ice/Long",
        "../Ice/HashMap",
        "../Ice/HashUtil",
        "../Ice/ArrayUtil",
        "../Ice/StreamHelpers",
        "../Ice/Identity",
        "../Ice/Endpoint"
    ]).Ice;

    const Slice = Ice.Slice;
/* slice2js browser-bundle-skip-end */

    /**
     * The batch compression option when flushing queued batch requests.
     *
     **/
    Ice.CompressBatch = Slice.defineEnum([
        ['Yes', 0], ['No', 1], ['BasedOnProxy', 2]]);

    /**
     * Base class providing access to the connection details.
     *
     **/
    Ice.ConnectionInfo = class
    {
        constructor(underlying = null, incoming = false, adapterName = "", connectionId = "")
        {
            this.underlying = underlying;
            this.incoming = incoming;
            this.adapterName = adapterName;
            this.connectionId = connectionId;
        }
    };


    /**
     * Specifies the close semantics for Active Connection Management.
     **/
    Ice.ACMClose = Slice.defineEnum([
        ['CloseOff', 0], ['CloseOnIdle', 1], ['CloseOnInvocation', 2], ['CloseOnInvocationAndIdle', 3], ['CloseOnIdleForceful', 4]]);

    /**
     * Specifies the heartbeat semantics for Active Connection Management.
     **/
    Ice.ACMHeartbeat = Slice.defineEnum([
        ['HeartbeatOff', 0], ['HeartbeatOnDispatch', 1], ['HeartbeatOnIdle', 2], ['HeartbeatAlways', 3]]);

    /**
     * A collection of Active Connection Management configuration settings.
     **/
    Ice.ACM = class
    {
        constructor(timeout = 0, close = Ice.ACMClose.CloseOff, heartbeat = Ice.ACMHeartbeat.HeartbeatOff)
        {
            this.timeout = timeout;
            this.close = close;
            this.heartbeat = heartbeat;
        }
    };

    Slice.defineStruct(Ice.ACM, true, true);

    /**
     * Determines the behavior when manually closing a connection.
     **/
    Ice.ConnectionClose = Slice.defineEnum([
        ['Forcefully', 0], ['Gracefully', 1], ['GracefullyWithWait', 2]]);

    /**
     * Provides access to the connection details of an IP connection
     *
     **/
    Ice.IPConnectionInfo = class extends Ice.ConnectionInfo
    {
        constructor(underlying, incoming, adapterName, connectionId, localAddress = "", localPort = -1, remoteAddress = "", remotePort = -1)
        {
            super(underlying, incoming, adapterName, connectionId);
            this.localAddress = localAddress;
            this.localPort = localPort;
            this.remoteAddress = remoteAddress;
            this.remotePort = remotePort;
        }
    };


    /**
     * Provides access to the connection details of a TCP connection
     *
     **/
    Ice.TCPConnectionInfo = class extends Ice.IPConnectionInfo
    {
        constructor(underlying, incoming, adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort, rcvSize = 0, sndSize = 0)
        {
            super(underlying, incoming, adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort);
            this.rcvSize = rcvSize;
            this.sndSize = sndSize;
        }
    };


    /**
     * Provides access to the connection details of a UDP connection
     *
     **/
    Ice.UDPConnectionInfo = class extends Ice.IPConnectionInfo
    {
        constructor(underlying, incoming, adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort, mcastAddress = "", mcastPort = -1, rcvSize = 0, sndSize = 0)
        {
            super(underlying, incoming, adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort);
            this.mcastAddress = mcastAddress;
            this.mcastPort = mcastPort;
            this.rcvSize = rcvSize;
            this.sndSize = sndSize;
        }
    };


    Slice.defineDictionary(Ice, "HeaderDict", "HeaderDictHelper", "Ice.StringHelper", "Ice.StringHelper", false, undefined, undefined);

    /**
     * Provides access to the connection details of a WebSocket connection
     *
     **/
    Ice.WSConnectionInfo = class extends Ice.ConnectionInfo
    {
        constructor(underlying, incoming, adapterName, connectionId, headers = null)
        {
            super(underlying, incoming, adapterName, connectionId);
            this.headers = headers;
        }
    };

/* slice2js browser-bundle-skip */
    exports.Ice = Ice;
/* slice2js browser-bundle-skip-end */
/* slice2js browser-bundle-skip */
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require :
 (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self.Ice._require : window.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports :
 (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self : window));
/* slice2js browser-bundle-skip-end */
