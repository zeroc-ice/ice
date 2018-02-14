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

    async function allTests(out, communicator, communicator2)
    {
        function test(value, ex)
        {
            if(!value)
            {
                let message = "test failed";
                if(ex)
                {
                    message += "\n" + ex.toString();
                }
                throw new Error(message);
            }
        }

        out.write("testing stringToProxy... ");
        let ref = "retry:default -p 12010";
        let base1 = communicator.stringToProxy(ref);
        test(base1 !== null);
        let base2 = communicator.stringToProxy(ref);
        test(base2 !== null);
        out.writeLine("ok");

        out.write("testing checked cast... ");
        let retry1 = await Test.RetryPrx.checkedCast(base1);
        test(retry1 !== null);
        test(retry1.equals(base1));
        let retry2 = await Test.RetryPrx.checkedCast(base2);
        test(retry2 !== null);
        test(retry2.equals(base2));
        out.writeLine("ok");

        out.write("calling regular operation with first proxy... ");
        await retry1.op(false);
        out.writeLine("ok");

        out.write("calling operation to kill connection with second proxy... ");
        try
        {
            await retry2.op(true);
            test(false);
        }
        catch(ex)
        {
            if(typeof(window) === 'undefined' && typeof(WorkerGlobalScope) === 'undefined') // Nodejs
            {
                test(ex instanceof Ice.ConnectionLostException, ex);
            }
            else // Browser
            {
                test(ex instanceof Ice.SocketException, ex);
            }
            out.writeLine("ok");
        }

        out.write("calling regular operation with first proxy again... ");
        await retry1.op(false);
        out.writeLine("ok");

        out.write("testing idempotent operation... ");
        let count = await retry1.opIdempotent(4);
        test(count === 4);
        out.writeLine("ok");

        out.write("testing non-idempotent operation... ");
        try
        {
            await retry1.opNotIdempotent();
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Ice.LocalException, ex);
        }
        out.writeLine("ok");

        out.write("testing invocation timeout and retries... ");
        retry2 = Test.RetryPrx.uncheckedCast(communicator2.stringToProxy(retry1.toString()));
        try
        {
            await retry2.ice_invocationTimeout(500).opIdempotent(4);
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Ice.InvocationTimeoutException, ex);
        }

        await retry2.opIdempotent(-1);
        out.writeLine("ok");

        await retry1.shutdown();
    }

    async function run(out, initData)
    {
        let communicator;
        let communicator2;
        try
        {
            //
            // For this test, we want to disable retries.
            //
            initData.properties.setProperty("Ice.RetryIntervals", "0 1 10 1");

            //
            // We don't want connection warnings because of the timeout
            //
            initData.properties.setProperty("Ice.Warn.Connections", "0");
            communicator = Ice.initialize(initData);

            //
            // Configure a second communicator for the invocation timeout
            // + retry test, we need to configure a large retry interval
            // to avoid time-sensitive failures.
            //
            let initData2 = new Ice.InitializationData();
            initData2.properties = communicator.getProperties().clone();
            initData2.properties.setProperty("Ice.RetryIntervals", "0 1 10000");
            communicator2 = Ice.initialize(initData2);

            await allTests(out, communicator, communicator2);
        }
        finally
        {
            if(communicator2)
            {
                await communicator2.destroy();
            }

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
