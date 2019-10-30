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
        "../Ice/Long",
        "../Ice/HashMap",
        "../Ice/HashUtil",
        "../Ice/ArrayUtil",
        "../Ice/StreamHelpers",
        "../Ice/Version",
        "../Ice/BuiltinSequences",
        "../Ice/EndpointF"
    ]).Ice;

    const Slice = Ice.Slice;
/* slice2js browser-bundle-skip-end */

    Object.defineProperty(Ice, 'TCPEndpointType', {
        value: 1
    });

    Object.defineProperty(Ice, 'SSLEndpointType', {
        value: 2
    });

    Object.defineProperty(Ice, 'UDPEndpointType', {
        value: 3
    });

    Object.defineProperty(Ice, 'WSEndpointType', {
        value: 4
    });

    Object.defineProperty(Ice, 'WSSEndpointType', {
        value: 5
    });

    Object.defineProperty(Ice, 'BTEndpointType', {
        value: 6
    });

    Object.defineProperty(Ice, 'BTSEndpointType', {
        value: 7
    });

    Object.defineProperty(Ice, 'iAPEndpointType', {
        value: 8
    });

    Object.defineProperty(Ice, 'iAPSEndpointType', {
        value: 9
    });

    /**
     * Base class providing access to the endpoint details.
     *
     **/
    Ice.EndpointInfo = class
    {
        constructor(underlying = null, timeout = 0, compress = false)
        {
            this.underlying = underlying;
            this.timeout = timeout;
            this.compress = compress;
        }
    };


    /**
     * Provides access to the address details of a IP endpoint.
     *
     * @see Endpoint
     *
     **/
    Ice.IPEndpointInfo = class extends Ice.EndpointInfo
    {
        constructor(underlying, timeout, compress, host = "", port = 0, sourceAddress = "")
        {
            super(underlying, timeout, compress);
            this.host = host;
            this.port = port;
            this.sourceAddress = sourceAddress;
        }
    };


    /**
     * Provides access to a TCP endpoint information.
     *
     * @see Endpoint
     *
     **/
    Ice.TCPEndpointInfo = class extends Ice.IPEndpointInfo
    {
        constructor(underlying, timeout, compress, host, port, sourceAddress)
        {
            super(underlying, timeout, compress, host, port, sourceAddress);
        }
    };


    /**
     * Provides access to an UDP endpoint information.
     *
     * @see Endpoint
     *
     **/
    Ice.UDPEndpointInfo = class extends Ice.IPEndpointInfo
    {
        constructor(underlying, timeout, compress, host, port, sourceAddress, mcastInterface = "", mcastTtl = 0)
        {
            super(underlying, timeout, compress, host, port, sourceAddress);
            this.mcastInterface = mcastInterface;
            this.mcastTtl = mcastTtl;
        }
    };


    /**
     * Provides access to a WebSocket endpoint information.
     *
     **/
    Ice.WSEndpointInfo = class extends Ice.EndpointInfo
    {
        constructor(underlying, timeout, compress, resource = "")
        {
            super(underlying, timeout, compress);
            this.resource = resource;
        }
    };


    /**
     * Provides access to the details of an opaque endpoint.
     *
     * @see Endpoint
     *
     **/
    Ice.OpaqueEndpointInfo = class extends Ice.EndpointInfo
    {
        constructor(underlying, timeout, compress, rawEncoding = new Ice.EncodingVersion(), rawBytes = null)
        {
            super(underlying, timeout, compress);
            this.rawEncoding = rawEncoding;
            this.rawBytes = rawBytes;
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
