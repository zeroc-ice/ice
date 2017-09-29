// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;

    async function run(communicator, prx, Test, bidir)
    {
        function test(value)
        {
            if(!value)
            {
                throw new Error("test failed");
            }
        }

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
            test(ex instanceof Ice.TwowayOnlyException);
        }

        try
        {
            await prx.ice_id();
            test(false);
        }
        catch(ex)
        {
            // Expected: twoway proxy required
            test(ex instanceof Ice.TwowayOnlyException);
        }

        try
        {
            await prx.ice_ids();
            test(false);
        }
        catch(ex)
        {
            // Expected: twoway proxy required
            test(ex instanceof Ice.TwowayOnlyException);
        }

        await prx.opVoid();

        await prx.opIdempotent();

        await prx.opNonmutating()

        try
        {
            await prx.opByte(0xff, 0x0f);
            test(false);
        }
        catch(ex)
        {
            // Expected: twoway proxy required
            test(ex instanceof Ice.TwowayOnlyException);
        }
    }

    exports.Oneways = { run: run };
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
