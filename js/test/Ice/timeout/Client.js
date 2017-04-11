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

    var allTests = function(out, communicator)
    {
        var failCB = function() { test(false); };
        var ref, obj, mult, timeout, to, connection, comm, now;

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
                out.write("testing connect timeout... ");
                to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(100 * mult));
                return timeout.holdAdapter(1000 * mult);
            }
        ).then(() => to.ice_getConnection()
        ).then(() => to.op() // Expect ConnectTimeoutException.
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Ice.ConnectTimeoutException);
                return timeout.op(); // Ensure adapter is active.
            }
        ).then(() =>
            {
                to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(1000 * mult));
                return timeout.holdAdapter(500 * mult);
            }
        ).then(() => to.ice_getConnection()
        ).then(() => to.op() // Expect success.
        ).then(() =>
            {
                out.writeLine("ok");
                out.write("testing connection timeout... ");
                to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(100 * mult));
                seq = new Uint8Array(10000000);
                return timeout.holdAdapter(1000 * mult);
            }
        ).then(() => to.sendData(seq) // Expect TimeoutException
        ).then(() => test(false),
               ex =>
            {
                test(ex instanceof Ice.TimeoutException);
                return timeout.op(); // Ensure adapter is active.
            }
        ).then(() =>
            {
                // NOTE: 30s timeout is necessary for Firefox/IE on Windows
                to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(30000 * mult));
                return timeout.holdAdapter(500 * mult);
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
                return to.sleep(750);
            }
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Ice.InvocationTimeoutException);
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
                return to.ice_getConnection();
            }
        ).then(con =>
            {
                connection = con;
                return timeout.holdAdapter(1500);
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
        ).delay(1000).then(() =>
            {
                try
                {
                    connection.getInfo();
                    test(false);
                }
                catch(ex)
                {
                    test(ex instanceof Ice.ConnectionManuallyClosedException); // Expected
                }
                return timeout.op();
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
                    initData.properties.setProperty("Ice.Override.Timeout", "100");
                }
                else
                {
                    initData.properties.setProperty("Ice.Override.Timeout", "2000");
                }
                comm = Ice.initialize(initData);
                return Test.TimeoutPrx.checkedCast(comm.stringToProxy(ref));
            }
        ).then(obj =>
            {
                to = obj;
                return timeout.holdAdapter(750 * 2 * mult);
            }
        ).then(() => to.sendData(seq) // Expect TimeoutException.
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Ice.TimeoutException);
                return timeout.op(); // Ensure adapter is active.
            }
        ).then(() => Test.TimeoutPrx.checkedCast(to.ice_timeout(1000 * mult)) // Calling ice_timeout() should have no effect.
        ).then(obj =>
            {
                to = obj;
                return timeout.holdAdapter(750 * 2 * mult);
            }
        ).then(() => to.sendData(seq) // Expect TimeoutException.
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Ice.TimeoutException);
                return comm.destroy();
            }
        ).then(() =>
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
        ).then(() => to.op()
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Ice.ConnectTimeoutException);
                return timeout.op(); // Ensure adapter is active.
            }
        ).then(() => timeout.holdAdapter(750 * mult)
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
                test(ex instanceof Ice.ConnectTimeoutException);
                return timeout.op(); // Ensure adapter is active.
            }
        ).then(() =>
            {
                to = Test.TimeoutPrx.uncheckedCast(to.ice_timeout(100 * mult));
                return to.ice_getConnection(); // Force connection.
            }
        ).then(obj => timeout.holdAdapter(750 * mult)
        ).then(() => to.sendData(seq)
        ).then(
            failCB,
            ex =>
            {
                test(ex instanceof Ice.TimeoutException);
                return comm.destroy();
            }
        ).then(() =>
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
        ).then(() => timeout.holdAdapter(500)
        ).then(() =>
            {
                now = Date.now();
                return comm.destroy();
            }
        ).then(() =>
            {
                var t = Date.now();
                test(t - now < 400);
                out.writeLine("ok");
                return timeout.shutdown();
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

        //
        // We need to send messages large enough to cause the transport
        // buffers to fill up.
        //
        id.properties.setProperty("Ice.MessageSizeMax", "10000");

        var c = Ice.initialize(id);
        return Ice.Promise.try(() =>
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
        ).finally(() => c.destroy());
    };
    exports._test = run;
    exports._runServer = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
