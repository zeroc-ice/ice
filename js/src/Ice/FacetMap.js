//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const _ModuleRegistry = require("../Ice/ModuleRegistry").Ice._ModuleRegistry;
const Ice = _ModuleRegistry.require(module,
                                    [
                                        "../Ice/Object",
                                        "../Ice/Value",
                                        "../Ice/Long",
                                        "../Ice/HashMap",
                                        "../Ice/HashUtil",
                                        "../Ice/ArrayUtil",
                                        "../Ice/StreamHelpers"
                                    ]).Ice;

const Slice = Ice.Slice;

Slice.defineDictionary(Ice, "FacetMap", "FacetMapHelper", "Ice.StringHelper", "Ice.ObjectHelper", false, undefined,
                       "Ice.Value");
exports.Ice = Ice;
