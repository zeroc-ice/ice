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

    var connect = function(prx)
    {
        var nRetry = 10;
        var next = function next() {
            if(--nRetry > 0) {
                return prx.ice_getConnection().then(c => c, ex => next());
            }
            return null;
        };
        return next();
    }

    var allTests = function(out, communicator)
    {
        var failCB = function() { test(false); };
        var ref, obj, mult, timeout, controller, to, connection, comm, now;

        var p = new Ice.Promise();
        var test = function(b, ex)
        {
            if(!b)
            {
                try
                {
                    var msg = "test failed";
                    if(ex)
                    {
                        msg += ":\n" + ex.toString();
                    }
                    throw new Error(msg);
                }
                catch(err)
                {
                    p.reject(err);
                    throw err;
                }
            }
        };

        var seq;
        Ice.Promise.try(() =>
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
        ).then(obj =>
            {
                timeout = obj;
                test(timeout !== null);

                controller = Test.ControllerPrx.uncheckedCast(communicator.stringToProxy("controller:default -p 12011"));
                test(controller !== null);

                out.write("testing connect timeout... ");
                to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(100 * mult));
                return controller.holdAdapter(-1);
            }
        ).then(() => to.ice_getConnection()
        ).then(() => to.op() // Expect ConnectTimeoutException.
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Ice.ConnectTimeoutException, ex);
                return controller.resumeAdapter().then(() => timeout.op());
            }
        ).then(() =>
            {
                to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(1000 * mult));
                return controller.holdAdapter(200 * mult);
            }
        ).then(() => to.ice_getConnection()
        ).then(() => to.op() // Expect success.
        ).then(() =>
            {
                out.writeLine("ok");
                out.write("testing connection timeout... ");
                to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(250 * mult));
                return connect(to);
            }
        ).then(() =>
            {
                seq = new Uint8Array(10000000);
                return controller.holdAdapter(-1);
            }
        ).then(() => to.sendData(seq) // Expect TimeoutException
        ).then(() => test(false),
               ex =>
            {
                test(ex instanceof Ice.TimeoutException, ex);
                return controller.resumeAdapter().then(() => timeout.op());
            }
        ).then(() =>
            {
                // NOTE: 30s timeout is necessary for Firefox/IE on Windows
                to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(30000 * mult));
                return controller.holdAdapter(200 * mult);
            }
        ).then(() => to.sendData(new Uint8Array(5 * 1024)) // Expect success.
        ).then(() =>
            {
                out.writeLine("ok");

                out.write("testing invocation timeout... ");
                return obj.ice_getConnection();
            }
        ).then(con =>
            {
                to = Test.TimeoutPrx.uncheckedCast(obj.ice_invocationTimeout(100));
                return to.ice_getConnection();
            }
        ).then(con =>
            {
                test(to.ice_getCachedConnection() === obj.ice_getCachedConnection());
                return to.sleep(500);
            }
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Ice.InvocationTimeoutException, ex);
                return obj.ice_ping();
            }
        ).then(() =>
            {
                to = Test.TimeoutPrx.uncheckedCast(obj.ice_invocationTimeout(500));
                return to.ice_getConnection();
            }
        ).then(con =>
            {
                test(to.ice_getCachedConnection() === obj.ice_getCachedConnection());
                return to.sleep(100);
            }
        ).then(() =>
            {
                out.writeLine("ok");
                out.write("testing close timeout... ");
                to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(500));
                return connect(to);
            }
        ).then(con =>
            {
                connection = con;
                return controller.holdAdapter(-1);
            }
        ).then(() => connection.close(Ice.ConnectionClose.GracefullyWithWait)
        ).then(() =>
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
        ).then(() =>
            {
                var loop = () => {
                    try
                    {
                        connection.getInfo();
                        return Ice.Promise.delay(10).then(loop);
                    }
                    catch(ex)
                    {
                        test(ex instanceof Ice.ConnectionManuallyClosedException, ex); // Expected
                        return controller.resumeAdapter().then(() => timeout.op());
                    }
                }
                return loop();
            }
        ).then(() =>
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
                    initData.properties.setProperty("Ice.Override.ConnectTimeout", "250");
                    initData.properties.setProperty("Ice.Override.Timeout", "100");
                }
                else
                {
                    initData.properties.setProperty("Ice.Override.ConnectTimeout", "5000");
                    initData.properties.setProperty("Ice.Override.Timeout", "2000");
                }
                comm = Ice.initialize(initData);
                to = Test.TimeoutPrx.uncheckedCast(comm.stringToProxy(ref));
                return connect(to);
            }
        ).then(c =>
            {
                return controller.holdAdapter(-1);
            }
        ).then(() => to.sendData(seq) // Expect TimeoutException.
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Ice.TimeoutException, ex);
                return controller.resumeAdapter().then(() => timeout.op());
            }
        ).then(() =>
            {
                to = Test.TimeoutPrx.uncheckedCast(to.ice_timeout(1000 * mult)) // Calling ice_timeout() should have no effect.
                return connect(to);
            }
        ).then(c =>
            {
                return controller.holdAdapter(-1);
            }
        ).then(() => to.sendData(seq) // Expect TimeoutException.
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Ice.TimeoutException, ex);
                return controller.resumeAdapter().then(() => timeout.op()).then(() => comm.destroy());
            }
        ).then(() => {
            // Small delay is useful for IE which doesn't like too many connection failures in a row
            return Ice.Promise.delay(100)
        }).then(() =>
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
                return controller.holdAdapter(-1);
            }
        ).then(() => to.op()
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Ice.ConnectTimeoutException, ex);
                return controller.resumeAdapter().then(() => timeout.op());
            }
        ).then(() => controller.holdAdapter(-1)
        ).then(() =>
            {
                //
                // Calling ice_timeout() should have no effect on the connect timeout.
                //
                to = Test.TimeoutPrx.uncheckedCast(to.ice_timeout(1000 * mult));
                return to.op();
            }
        ).then(() => test(false),
               ex =>
            {
                test(ex instanceof Ice.ConnectTimeoutException, ex);
                return controller.resumeAdapter().then(() => timeout.op());
            }
        ).then(() =>
            {
                to = Test.TimeoutPrx.uncheckedCast(to.ice_timeout(100 * mult));
                return connect(to); // Force connection.
            }
        ).then(obj => controller.holdAdapter(-1)
        ).then(() => to.sendData(seq)
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Ice.TimeoutException, ex);
                return controller.resumeAdapter().then(() => timeout.op()).then(() => comm.destroy());
            }
        ).then(() => {
            // Small delay is useful for IE which doesn't like too many connection failures in a row
            return Ice.Promise.delay(100)
        }).then(() =>
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
        ).then(() => controller.holdAdapter(-1)
        ).then(() =>
            {
                now = Date.now();
                return comm.destroy();
            }
        ).then(() =>
            {
                var t = Date.now();
                test(t - now < 400);
                return controller.resumeAdapter();
            }
        ).then(() =>
            {
                out.writeLine("ok");
                return controller.shutdown();
            }
        ).then(p.resolve, p.reject);
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

        id.properties.setProperty("Ice.PrintStackTraces", "1");

        var c = Ice.initialize(id);
        return Ice.Promise.try(() => allTests(out, c)).finally(() => c.destroy());
    };
    exports._test = run;
    exports._runServer = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
