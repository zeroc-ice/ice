//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

/* eslint-disable */
/* jshint ignore: start */

/* slice2js browser-bundle-skip */
const _ModuleRegistry = require("../Ice/ModuleRegistry").Ice._ModuleRegistry;
require("../Ice/Object");
require("../Ice/Value");
require("../Ice/ObjectPrx");
require("../Ice/Long");
require("../Ice/HashMap");
require("../Ice/HashUtil");
require("../Ice/ArrayUtil");
require("../Ice/StreamHelpers");
require("../Ice/Version");
require("../Ice/BuiltinSequences");
require("../Ice/EndpointF");
const Ice = _ModuleRegistry.module("Ice");

const Slice = Ice.Slice;
/* slice2js browser-bundle-skip-end */

/**
 *  Base class providing access to the endpoint details.
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
 *  Provides access to the address details of a IP endpoint.
 *  @see Endpoint
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
 *  Provides access to a TCP endpoint information.
 *  @see Endpoint
 **/
Ice.TCPEndpointInfo = class extends Ice.IPEndpointInfo
{
    constructor(underlying, timeout, compress, host, port, sourceAddress)
    {
        super(underlying, timeout, compress, host, port, sourceAddress);
    }
};

/**
 *  Provides access to a WebSocket endpoint information.
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
 *  Provides access to the details of an opaque endpoint.
 *  @see Endpoint
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