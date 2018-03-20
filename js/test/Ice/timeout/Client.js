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

    async function allTests(out, communicator, initData)
    {
        function test(value, ex)
        {
            if(!value)
            {
                let error = "test failed";
                if(ex)
                {
                    error += "\n" + ex.toString();
                }
                throw new Error(error);
            }
        }

        async function connect(prx)
        {
            let nRetry = 10;
            while(--nRetry > 0)
            {
                try
                {
                    await prx.ice_getConnection();
                    break;
                }
                catch(ex)
                {
                    // Can sporadically occur with slow machines
                    test(ex instanceof Ice.ConnectTimeoutException ||
                         ex instanceof Ice.ConnectFailedException, ex);
                }
                await Ice.Promise.delay(100);
            }
            return await prx.ice_getConnection();
        }

        let ref = "timeout:default -p 12010";
        let obj = communicator.stringToProxy(ref);
        test(obj !== null);

        let mult = 1;
        if(["ssl", "wss"].includes(communicator.getProperties().getPropertyWithDefault("Ice.Default.Protocol", "tcp")))
        {
            mult = 4;
        }

        let timeout = await Test.TimeoutPrx.checkedCast(obj);
        test(timeout !== null);

        let controller = Test.ControllerPrx.uncheckedCast(communicator.stringToProxy("controller:default -p 12011"));
        test(controller !== null);

        out.write("testing connect timeout... ");
        {
            let to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(100 * mult));
            await controller.holdAdapter(-1);
            try
            {
                await to.op(); // Expect ConnectTimeoutException.
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.ConnectTimeoutException, ex);
            }
            await controller.resumeAdapter();
            await timeout.op(); // Ensure adapter is active.
        }

        {
            let to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(1000 * mult));
            await controller.holdAdapter(200 * mult);
            await to.ice_getConnection();
            try
            {
                await to.op(); // Expect success.
            }
            catch(ex)
            {
                test(false, ex);
            }
        }
        out.writeLine("ok");

        let seq = new Uint8Array(10000000);
        out.write("testing connection timeout... ");
        {
            let to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(250 * mult));
            await connect(to);
            await controller.holdAdapter(-1);
            try
            {
                await to.sendData(seq); // Expect TimeoutException
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.TimeoutException, ex);
            }
            await controller.resumeAdapter();
            await timeout.op(); // Ensure adapter is active.
        }

        {
            // NOTE: 30s timeout is necessary for Firefox/IE on Windows
            let to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(30000 * mult));
            await controller.holdAdapter(200 * mult);
            try
            {
                await to.sendData(new Uint8Array(5 * 1024)); // Expect success.
            }
            catch(ex)
            {
                test(false, ex);
            }
        }
        out.writeLine("ok");

        out.write("testing invocation timeout... ");
        {
            let connection = await obj.ice_getConnection();
            let to = Test.TimeoutPrx.uncheckedCast(obj.ice_invocationTimeout(100));
            test(connection == await to.ice_getConnection());

            try
            {
                await to.sleep(500);
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.InvocationTimeoutException, ex);
            }
            await obj.ice_ping();
            to = await Test.TimeoutPrx.checkedCast(obj.ice_invocationTimeout(1000));
            test(connection === await obj.ice_getConnection());

            try
            {
                await to.sleep(100);
            }
            catch(ex)
            {
                test(ex instanceof Ice.InvocationTimeoutException, ex);
            }
        }
        out.writeLine("ok");

        out.write("testing close timeout... ");
        {
            let to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(500));
            let connection = await connect(to);
            await controller.holdAdapter(-1);
            await connection.close(Ice.ConnectionClose.GracefullyWithWait);

            try
            {
                connection.getInfo(); // getInfo() doesn't throw in the closing state
            }
            catch(ex)
            {
                test(false, ex);
            }

            while(true)
            {
                try
                {
                    connection.getInfo();
                    await Ice.Promise.delay(10);
                }
                catch(ex)
                {
                    test(ex instanceof Ice.ConnectionManuallyClosedException, ex); // Expected
                    test(ex.graceful);
                    break;
                }
            }
            await controller.resumeAdapter();
            await timeout.op();
        }
        out.writeLine("ok");

        // Small delay is useful for IE which doesn't like too many connection failures in a row
        await Ice.Promise.delay(500);

        out.write("testing timeout overrides... ");
        {
            //
            // Test Ice.Override.Timeout. This property overrides all
            // endpoint timeouts.
            //
            let initData = new Ice.InitializationData();
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
            let comm = Ice.initialize(initData);
            let to = Test.TimeoutPrx.uncheckedCast(comm.stringToProxy(ref));
            await connect(to);
            await controller.holdAdapter(-1);

            try
            {
                await to.sendData(seq) // Expect TimeoutException.
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.TimeoutException, ex);
            }

            await controller.resumeAdapter();
            await timeout.op();
            //
            // Calling ice_timeout() should have no effect.
            //
            to = Test.TimeoutPrx.uncheckedCast(to.ice_timeout(1000 * mult));
            await connect(to);
            await controller.holdAdapter(-1);
            try
            {
                await to.sendData(seq) // Expect TimeoutException.
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.TimeoutException, ex);
            }
            await controller.resumeAdapter();
            await timeout.op();
            await comm.destroy();
        }

        // Small delay is useful for IE which doesn't like too many connection failures in a row
        await Ice.Promise.delay(500);

        {
            //
            // Test Ice.Override.ConnectTimeout.
            //
            let initData = new Ice.InitializationData();
            initData.properties = communicator.getProperties().clone();
            if(mult === 1)
            {
                initData.properties.setProperty("Ice.Override.ConnectTimeout", "250");
            }
            else
            {
                initData.properties.setProperty("Ice.Override.ConnectTimeout", "1000");
            }
            let comm = Ice.initialize(initData);
            to = Test.TimeoutPrx.uncheckedCast(comm.stringToProxy(ref));
            await controller.holdAdapter(-1);

            try
            {
                await to.op();
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.ConnectTimeoutException, ex);
            }

            await controller.resumeAdapter();
            await timeout.op();
            await controller.holdAdapter(-1);

            //
            // Calling ice_timeout() should have no effect on the connect timeout.
            //
            to = Test.TimeoutPrx.uncheckedCast(to.ice_timeout(1000 * mult));

            try
            {
                await to.op();
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.ConnectTimeoutException, ex);
            }
            await controller.resumeAdapter();
            await timeout.op(); // Ensure adapter is active

            //
            // Verify that timeout set via ice_timeout() is still used for requests.
            //
            to = Test.TimeoutPrx.uncheckedCast(to.ice_timeout(100 * mult));
            await connect(to); // Force connection.
            await controller.holdAdapter(-1);

            try
            {
                await to.sendData(seq);
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.TimeoutException, ex);
            }
            await controller.resumeAdapter();
            await timeout.op();
            await comm.destroy();
        }

        // Small delay is useful for IE which doesn't like too many connection failures in a row
        await Ice.Promise.delay(500);

        {
            //
            // Test Ice.Override.CloseTimeout.
            //
            let initData = new Ice.InitializationData();
            initData.properties = communicator.getProperties().clone();
            initData.properties.setProperty("Ice.Override.CloseTimeout", "100");
            let comm = Ice.initialize(initData);
            await comm.stringToProxy(ref).ice_getConnection();

            await controller.holdAdapter(-1);

            let start = Date.now();
            await comm.destroy();
            let end = Date.now();
            test(end - start < 1000);
            await controller.resumeAdapter();
            out.writeLine("ok");
            await controller.shutdown();
        }
    }

    async function run(out, initData)
    {
        let communicator;
        try
        {
            //
            // For this test, we want to disable retries.
            //
            initData.properties.setProperty("Ice.RetryIntervals", "-1");

            //
            // We don't want connection warnings because of the timeout
            //
            initData.properties.setProperty("Ice.Warn.Connections", "0");

            initData.properties.setProperty("Ice.PrintStackTraces", "1");

            communicator = Ice.initialize(initData);
            await allTests(out, communicator, initData);
        }
        finally
        {
            if(communicator)
            {
                await communicator.destroy();
            }
        }
    }

    exports._test = run;
    exports._runServer = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
