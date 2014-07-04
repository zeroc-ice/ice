// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    var require = typeof(module) !== "undefined" ? module.require : function(){};
    require("Ice/Ice");
    var Ice = global.Ice;

    require("Test");
    var Test = global.Test;
    var Promise = Ice.Promise;

    var allTests = function(out, communicator)
    {
        var ref, base1, base2, retry1, retry2;

        var p = new Promise();
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
                    p.fail(err);
                    throw err;
                }
            }
        };

        Promise.try(
            function()
            {
                out.write("testing stringToProxy... ");
                ref = "retry:default -p 12010";
                base1 = communicator.stringToProxy(ref);
                test(base1 !== null);
                base2 = communicator.stringToProxy(ref);
                test(base2 !== null);
                out.writeLine("ok");
                out.write("testing checked cast... ");
                return Test.RetryPrx.checkedCast(base1);
            }
        ).then(
            function(obj)
            {
                retry1 = obj;
                test(retry1 !== null);
                test(retry1.equals(base1));
                return Test.RetryPrx.checkedCast(base2);
            }
        ).then(
            function(obj)
            {
                retry2 = obj;
                test(retry2 !== null);
                test(retry2.equals(base2));
                out.writeLine("ok");
                out.write("calling regular operation with first proxy... ");
                return retry1.op(false);
            }
        ).then(
            function()
            {
                out.writeLine("ok");
                out.write("calling operation to kill connection with second proxy... ");
                return retry2.op(true);
            }
        ).then(
            function()
            {
                test(false);
            },
            function(ex)
            {
                test((typeof(window) === undefined && ex instanceof Ice.ConnectionLostException) ||
                        (typeof(window) !== undefined && ex instanceof Ice.SocketException));
                out.writeLine("ok");
                out.write("calling regular operation with first proxy again... ");
                return retry1.op(false);
            }
        ).then(
            function()
            {
                out.writeLine("ok");
                out.write("testing idempotent operation... ");
                return retry1.opIdempotent(0);
            }
        ).then(
            function(count)
            {
                test(count === 4);
                out.writeLine("ok");
                out.write("testing non-idempotent operation... ");
                return retry1.opNotIdempotent(4);
            }
        ).then(
            function()
            {
                test(false);
            },
            function(ex)
            {
                test(ex instanceof Ice.LocalException);
                return retry1.shutdown();
            }
        ).then(
            function()
            {
                out.writeLine("ok");
                p.succeed();
            },
            function(ex)
            {
                p.fail(ex);
            }
        );
        return p;
    };

    var run = function(out, id)
    {
        return Promise.try(
            function()
            {
                //
                // For this test, we want to disable retries.
                //
                id.properties.setProperty("Ice.RetryIntervals", "0 10 20 30");

                //
                // We don't want connection warnings because of the timeout
                //
                id.properties.setProperty("Ice.Warn.Connections", "0");
                var c = Ice.initialize(id);
                return allTests(out, c).finally(
                    function()
                    {
                        if(c)
                        {
                            return c.destroy();
                        }
                    });
            });
    };
    global.__test__ = run;
    global.__runServer__ = true;
}(typeof (global) === "undefined" ? window : global));
