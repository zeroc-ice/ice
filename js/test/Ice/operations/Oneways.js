 // **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

(function(module, require, exports)
{
    const Ice = require("ice").Ice;
    const test = require("TestHelper").TestHelper.test;

    async function run(communicator, prx, Test, bidir)
    {
        prx = prx.ice_oneway();
        await prx.ice_ping();

        try
        {
            await prx.ice_isA(Test.MyClass.ice_staticId());
            test(false);
        }
        catch(ex)
        {
            // Expected: twoway proxy required
            test(ex instanceof Ice.TwowayOnlyException, ex);
        }

        try
        {
            await prx.ice_id();
            test(false);
        }
        catch(ex)
        {
            // Expected: twoway proxy required
            test(ex instanceof Ice.TwowayOnlyException, ex);
        }

        try
        {
            await prx.ice_ids();
            test(false);
        }
        catch(ex)
        {
            // Expected: twoway proxy required
            test(ex instanceof Ice.TwowayOnlyException, ex);
        }

        await prx.opVoid();
        await prx.opIdempotent();
        await prx.opNonmutating();

        try
        {
            await prx.opByte(0xff, 0x0f);
            test(false);
        }
        catch(ex)
        {
            // Expected: twoway proxy required
            test(ex instanceof Ice.TwowayOnlyException, ex);
        }
    }

    exports.Oneways = {run: run};
}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self.Ice._require : window.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self : window));
