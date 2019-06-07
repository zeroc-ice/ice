//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

(function(module, require, exports)
{
    const M = require("Test").M;

    class AMDBI extends M.BDisp
    {
        opB()
        {
        }

        opIntf()
        {
        }
    }
    exports.AMDBI = AMDBI;
}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self.Ice._require : window.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self : window));
