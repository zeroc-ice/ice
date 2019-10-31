//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const _ModuleRegistry = require("../Ice/ModuleRegistry").Ice._ModuleRegistry;
const Ice = _ModuleRegistry.require(module,
                                    [
                                        "../Ice/EnumBase",
                                        "../Ice/Long",
                                        "../Ice/HashMap",
                                        "../Ice/HashUtil",
                                        "../Ice/ArrayUtil",
                                        "../Ice/StreamHelpers"
                                    ]).Ice;

const Slice = Ice.Slice;

/**
 * Determines the order in which the Ice run time uses the endpoints
 * in a proxy when establishing a connection.
 **/
Ice.EndpointSelectionType = Slice.defineEnum([
    ['Random', 0], ['Ordered', 1]]);

exports.Ice = Ice;
