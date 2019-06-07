//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

(function(module, require, exports)
{
    const M = require("Test").M;

    class BI extends M.BDisp
    {
        opB()
        {
        }

        opIntf()
        {
        }
    }
    exports.BI = BI;
}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self.Ice._require : window.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self : window));
