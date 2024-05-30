//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

/* eslint-disable */
/* jshint ignore: start */

/* slice2js browser-bundle-skip */
const _ModuleRegistry = require("../Ice/ModuleRegistry").Ice._ModuleRegistry;
require("../Ice/Struct");
require("../Ice/Long");
require("../Ice/HashMap");
require("../Ice/HashUtil");
require("../Ice/ArrayUtil");
require("../Ice/StreamHelpers");
require("../Ice/Context");
require("../Ice/Identity");
require("../Ice/OperationMode");
require("../Ice/Version");
const Ice = _ModuleRegistry.module("Ice");

const Slice = Ice.Slice;
/* slice2js browser-bundle-skip-end */

/**
 *  Information about the current method invocation for servers. Each operation on the server has a
 *  <code>Current</code> as its implicit final parameter. <code>Current</code> is mostly used for Ice services. Most
 *  applications ignore this parameter.
 **/
Ice.Current = class
{
    constructor(adapter = null, con = null, id = new Ice.Identity(), facet = "", operation = "", mode = Ice.OperationMode.Normal, ctx = null, requestId = 0, encoding = new Ice.EncodingVersion())
    {
        this.adapter = adapter;
        this.con = con;
        this.id = id;
        this.facet = facet;
        this.operation = operation;
        this.mode = mode;
        this.ctx = ctx;
        this.requestId = requestId;
        this.encoding = encoding;
    }
};

Slice.defineStruct(Ice.Current, false, true);
/* slice2js browser-bundle-skip */
exports.Ice = Ice;
/* slice2js browser-bundle-skip-end */
