//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const _ModuleRegistry = require("../Ice/ModuleRegistry").Ice._ModuleRegistry;
const Ice = _ModuleRegistry.require(module,
[
    "../Ice/Struct",
    "../Ice/EnumBase",
    "../Ice/Long",
    "../Ice/HashMap",
    "../Ice/HashUtil",
    "../Ice/ArrayUtil",
    "../Ice/StreamHelpers",
    "../Ice/Identity",
    "../Ice/Version"
]).Ice;

const Slice = Ice.Slice;

Slice.defineDictionary(Ice, "Context", "ContextHelper", "Ice.StringHelper", "Ice.StringHelper", false, undefined, undefined);

/**
 * Determines the retry behavior an invocation in case of a (potentially) recoverable error.
 *
 **/
Ice.OperationMode = Slice.defineEnum([
    ['Normal', 0], ['Nonmutating', 1], ['Idempotent', 2]]);

/**
 * Information about the current method invocation for servers. Each
 * operation on the server has a <code>Current</code> as its implicit final
 * parameter. <code>Current</code> is mostly used for Ice services. Most
 * applications ignore this parameter.
 *
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
