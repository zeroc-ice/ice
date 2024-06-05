//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

/* eslint-disable */
/* jshint ignore: start */

/* slice2js browser-bundle-skip */
const _ModuleRegistry = require("../Ice/ModuleRegistry").Ice._ModuleRegistry;
require("../Ice/EnumBase");
require("../Ice/Long");
require("../Ice/HashMap");
require("../Ice/HashUtil");
require("../Ice/ArrayUtil");
require("../Ice/StreamHelpers");
const Ice = _ModuleRegistry.module("Ice");

const Slice = Ice.Slice;
/* slice2js browser-bundle-skip-end */

/**
 *  Determines the order in which the Ice run time uses the endpoints in a proxy when establishing a connection.
 **/
Ice.EndpointSelectionType = Slice.defineEnum([
    ['Random', 0], ['Ordered', 1]]);
/* slice2js browser-bundle-skip */
exports.Ice = Ice;
/* slice2js browser-bundle-skip-end */