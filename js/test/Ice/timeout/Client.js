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
    var Promise = Ice.Promise;

    var allTests = function(out, communicator)
    {
        var failCB = function() { test(false); };
        var ref, obj, mult, timeout, to, connection, comm, now;

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

        var seq;
        Promise.try(
            function()
            {
                ref = "timeout:default -p 12010";
                obj = communicator.stringToProxy(ref);
                test(obj !== null);

                mult = 1;
                if(communicator.getProperties().getPropertyWithDefault("Ice.Default.Protocol", "tcp") === "ssl" ||
                   communicator.getProperties().getPropertyWithDefault("Ice.Default.Protocol", "tcp") === "wss")
                {
                    mult = 4;
                }

                return Test.TimeoutPrx.checkedCast(obj);
            }
        ).then(
            function(obj)
            {
                timeout = obj;
                test(timeout !== null);
                out.write("testing connect timeout... ");
                to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(100 * mult));
                return timeout.holdAdapter(1000 * mult);
            }
        ).then(
            function()
            {
                return to.ice_getConnection();
            }
        ).then(
            function()
            {
                //
                // Expect ConnectTimeoutException.
                //
                return to.op();
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Ice.ConnectTimeoutException);
                return timeout.op(); // Ensure adapter is active.
            }
        ).then(
            function()
            {
                to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(1000 * mult));
                return timeout.holdAdapter(500 * mult);
            }
        ).then(
            function()
            {
                return to.ice_getConnection();
            }
        ).then(
            function()
            {
                //
                // Expect success.
                //
                return to.op();
            }
        ).then(
            function()
            {
                out.writeLine("ok");
                out.write("testing connection timeout... ");
                to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(100 * mult));
                seq = Ice.Buffer.createNative(new Array(10000000));
                return timeout.holdAdapter(1000 * mult);
            }
        ).then(
            function()
            {
                //
                // Expect TimeoutException.
                //
                return to.sendData(seq);
            }
        ).then(
            function()
            {
                test(false);
            },
            function(ex)
            {
                test(ex instanceof Ice.TimeoutException);
                return timeout.op(); // Ensure adapter is active.
            }
        ).then(
            function()
            {
                // NOTE: 30s timeout is necessary for Firefox/IE on Windows
                to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(30000 * mult));
                return timeout.holdAdapter(500 * mult);
            }
        ).then(
            function()
            {
                //
                // Expect success.
                //
                return to.sendData(Ice.Buffer.createNative(new Array(5 * 1024)));
            }
        ).then(
            function()
            {
                out.writeLine("ok");

                out.write("testing invocation timeout... ");
                return obj.ice_getConnection();
            }
        ).then(
            function(con)
            {
                to = Test.TimeoutPrx.uncheckedCast(obj.ice_invocationTimeout(100));
                return to.ice_getConnection();
            }
        ).then(
            function(con)
            {
                test(to.ice_getCachedConnection() === obj.ice_getCachedConnection());
                return to.sleep(750);
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Ice.InvocationTimeoutException);
                return obj.ice_ping();
            }
        ).then(
            function()
            {
                to = Test.TimeoutPrx.uncheckedCast(obj.ice_invocationTimeout(500));
                return to.ice_getConnection();
            }
        ).then(
            function(con)
            {
                test(to.ice_getCachedConnection() === obj.ice_getCachedConnection());
                return to.sleep(250);
            }
        ).then(
            function()
            {
                out.writeLine("ok");
                out.write("testing close timeout... ");
                to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(500));
                return to.ice_getConnection();
            }
        ).then(
            function(con)
            {
                connection = con;
                return timeout.holdAdapter(1500);
            }
        ).then(
            function()
            {
                return connection.close(false);
            }
        ).then(
            function()
            {
                try
                {
                    connection.getInfo(); // getInfo() doesn't throw in the closing state
                }
                catch(ex)
                {
                    test(false);
                }
            }
        ).delay(1000).then(
            function()
            {
                try
                {
                    connection.getInfo();
                    test(false);
                }
                catch(ex)
                {
                    test(ex instanceof Ice.CloseConnectionException); // Expected
                }
                return timeout.op();
            }
        ).then(
            function()
            {
                out.writeLine("ok");
                out.write("testing timeout overrides... ");
                //
                // Test Ice.Override.Timeout. This property overrides all
                // endpoint timeouts.
                //
                var initData = new Ice.InitializationData();
                initData.properties = communicator.getProperties().clone();
                if(mult === 1)
                {
                    initData.properties.setProperty("Ice.Override.Timeout", "100");
                }
                else
                {
                    initData.properties.setProperty("Ice.Override.Timeout", "2000");
                }
                comm = Ice.initialize(initData);
                return Test.TimeoutPrx.checkedCast(comm.stringToProxy(ref));
            }
        ).then(
            function(obj)
            {
                to = obj;
                return timeout.holdAdapter(750 * 2 * mult);
            }
        ).then(
            function()
            {
                //
                // Expect TimeoutException.
                //
                return to.sendData(seq);
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Ice.TimeoutException);
                return timeout.op(); // Ensure adapter is active.
            }
        ).then(
            function()
            {
                //
                // Calling ice_timeout() should have no effect.
                //
                return Test.TimeoutPrx.checkedCast(to.ice_timeout(1000 * mult));
            }
        ).then(
            function(obj)
            {
                to = obj;
                return timeout.holdAdapter(750 * 2 * mult);
            }
        ).then(
            function()
            {
                //
                // Expect TimeoutException.
                //
                return to.sendData(seq);
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Ice.TimeoutException);
                return comm.destroy();
            }
        ).then(
            function()
            {
                //
                // Test Ice.Override.ConnectTimeout.
                //
                var initData = new Ice.InitializationData();
                initData.properties = communicator.getProperties().clone();
                if(mult === 1)
                {
                    initData.properties.setProperty("Ice.Override.ConnectTimeout", "250");
                }
                else
                {
                    initData.properties.setProperty("Ice.Override.ConnectTimeout", "1000");
                }
                comm = Ice.initialize(initData);
                to = Test.TimeoutPrx.uncheckedCast(comm.stringToProxy(ref));
                return timeout.holdAdapter(750 * mult);
            }
        ).then(
            function()
            {
                return to.op();
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Ice.ConnectTimeoutException);
                return timeout.op(); // Ensure adapter is active.
            }
        ).then(
            function()
            {
                return timeout.holdAdapter(750 * mult);
            }
        ).then(
            function()
            {
                //
                // Calling ice_timeout() should have no effect on the connect timeout.
                //
                to = Test.TimeoutPrx.uncheckedCast(to.ice_timeout(1000 * mult));
                return to.op();
            }
        ).then(
            function()
            {
                test(false);
            },
            function(ex)
            {
                test(ex instanceof Ice.ConnectTimeoutException);
                return timeout.op(); // Ensure adapter is active.
            }
        ).then(
            function()
            {
                to = Test.TimeoutPrx.uncheckedCast(to.ice_timeout(100 * mult));
                return to.ice_getConnection(); // Force connection.
            }
        ).then(
            function(obj)
            {
                return timeout.holdAdapter(750 * mult);
            }
        ).then(
            function()
            {
                return to.sendData(seq);
            }
        ).then(
            failCB,
            function(ex)
            {
                test(ex instanceof Ice.TimeoutException);
                return comm.destroy();
            }
        ).then(
            function()
            {
                //
                // Test Ice.Override.CloseTimeout.
                //
                var initData = new Ice.InitializationData();
                initData.properties = communicator.getProperties().clone();
                initData.properties.setProperty("Ice.Override.CloseTimeout", "100");
                comm = Ice.initialize(initData);
                return comm.stringToProxy(ref).ice_getConnection();
            }
        ).then(
            function(con)
            {
                return timeout.holdAdapter(500);
            }
        ).then(
            function()
            {
                now = Date.now();
                return comm.destroy();
            }
        ).then(
            function()
            {
                var t = Date.now();
                test(t - now < 400);
                out.writeLine("ok");
                return timeout.shutdown();
            }
        ).then(
            function()
            {
                p.succeed();
            });
        return p;
    };

    var run = function(out, id)
    {
        //
        // For this test, we want to disable retries.
        //
        id.properties.setProperty("Ice.RetryIntervals", "-1");

        //
        // We don't want connection warnings because of the timeout
        //
        id.properties.setProperty("Ice.Warn.Connections", "0");

        //
        // We need to send messages large enough to cause the transport
        // buffers to fill up.
        //
        id.properties.setProperty("Ice.MessageSizeMax", "10000");

        var c = Ice.initialize(id);
        return Promise.try(
            function()
            {
                if(typeof(navigator) !== 'undefined' && isSafari() && isWorker())
                {
                    out.writeLine("Test not supported with Safari web workers.");
                    return Test.TimeoutPrx.uncheckedCast(c.stringToProxy("timeout:default -p 12010")).shutdown();
                }
                else
                {
                    return allTests(out, c);
                }
            }
        ).finally(
            function()
            {
                return c.destroy();
            }
        );
    };
    exports.__test__ = run;
    exports.__runServer__ = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
