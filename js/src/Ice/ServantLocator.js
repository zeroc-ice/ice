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
        "../Ice/Current"
    ]).Ice;

    const Slice = Ice.Slice;
/* slice2js browser-bundle-skip-end */
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
