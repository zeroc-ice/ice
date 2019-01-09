// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

(function(module, require, exports)
{
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;
    const TestHelper = require("TestHelper").TestHelper;
    const test = TestHelper.test;

    class Client extends TestHelper
    {
        async allTests(communicator, communicator2)
        {
            const out = this.getWriter();
            out.write("testing stringToProxy... ");
            const ref = "retry:" + this.getTestEndpoint();
            const base1 = communicator.stringToProxy(ref);
            test(base1 !== null);
            const base2 = communicator.stringToProxy(ref);
            test(base2 !== null);
            out.writeLine("ok");

            out.write("testing checked cast... ");
            const retry1 = await Test.RetryPrx.checkedCast(base1);
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
                if(typeof window === 'undefined' && typeof WorkerGlobalScope === 'undefined') // Nodejs
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
            const count = await retry1.opIdempotent(4);
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

        async run(args)
        {
            let communicator;
            let communicator2;
            try
            {
                let [properties] = this.createTestProperties(args);

                //
                // For this test, we want to disable retries.
                //
                properties.setProperty("Ice.RetryIntervals", "0 1 10 1");

                //
                // We don't want connection warnings because of the timeout
                //
                properties.setProperty("Ice.Warn.Connections", "0");

                [communicator] = this.initialize(properties);

                //
                // Configure a second communicator for the invocation timeout
                // + retry test, we need to configure a large retry interval
                // to avoid time-sensitive failures.
                //
                properties = communicator.getProperties().clone();
                properties.setProperty("Ice.RetryIntervals", "0 1 10000");
                [communicator2] = this.initialize(properties);

                await this.allTests(communicator, communicator2);
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
    }
    exports.Client = Client;
}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self.Ice._require : window.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self : window));
