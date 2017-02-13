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
    var Ice = require("ice").Ice;
    var Test = require("Test").Test;

    var run = function(communicator, prx, Test, bidir)
    {
        var p = new Ice.Promise();
        var test = function(b)
        {
            if(!b)
            {
                try
                {
                    throw new Error("test failed");
                }
                catch(err)
                {
                    p.reject(err);
                    throw err;
                }
            }
        };

        Ice.Promise.try(() =>
            {
                prx = prx.ice_oneway();
                return prx.ice_ping();
            }
        ).then(() =>
            {
                try
                {
                    prx.ice_isA(Test.MyClass.ice_staticId());
                    test(false);
                }
                catch(ex)
                {
                    // Expected: twoway proxy required
                }

                try
                {
                    prx.ice_id();
                    test(false);
                }
                catch(ex)
                {
                    // Expected: twoway proxy required
                }

                try
                {
                    prx.ice_ids();
                    test(false);
                }
                catch(ex)
                {
                    // Expected: twoway proxy required
                }

                return prx.opVoid();
            }
        ).then(() => prx.opIdempotent()
        ).then(() => prx.opNonmutating()
        ).then(() =>
            {
                try
                {
                    prx.opByte(0xff, 0x0f);
                    test(false);
                }
                catch(ex)
                {
                    // Expected: twoway proxy required
                }
            }
        ).then(p.resolve, p.reject);
        return p;
    };

    exports.Oneways = { run: run };
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));

