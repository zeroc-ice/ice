// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
        function test(value, ex)
        {
            if(!value)
            {
                let message = "test failed";
                if(ex)
                {
                    message + "\n" + ex.toString();
                }
                throw new Error(message);
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

        await prx.opNonmutating()

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

    exports.Oneways = { run: run };
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
