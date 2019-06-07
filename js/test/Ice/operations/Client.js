//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

(function(module, require, exports)
{
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;
    const M = require("Test").M;
    const TestHelper = require("TestHelper").TestHelper;
    const Twoways = require("Twoways").Twoways;
    const Oneways = require("Oneways").Oneways;
    const BatchOneways = require("BatchOneways").BatchOneways;

    class Client extends TestHelper
    {
        async allTests(Test, bidir)
        {
            const out = this.getWriter();
            const communicator = this.communicator();
            out.write("testing twoway operations... ");
            const ref = "test:" + this.getTestEndpoint();
            const base = communicator.stringToProxy(ref);
            const cl = await Test.MyClassPrx.checkedCast(base);
            const derived = await Test.MyDerivedClassPrx.checkedCast(cl);

            await Twoways.run(communicator, cl, Test, M, bidir, this);
            await Twoways.run(communicator, derived, Test, M, bidir, this);
            out.writeLine("ok");

            out.write("testing oneway operations... ");
            await Oneways.run(communicator, cl, Test, bidir);
            out.writeLine("ok");

            out.write("testing batch oneway operations... ");
            await BatchOneways.run(communicator, cl, Test, bidir);
            out.writeLine("ok");

            out.write("testing server shutdown... ");
            await cl.shutdown();
            try
            {
                await cl.ice_timeout(100).ice_ping(); // Use timeout to speed up testing on Windows
                throw new Error("test failed");
            }
            catch(ex)
            {
                if(ex instanceof Ice.LocalException)
                {
                    out.writeLine("ok");
                }
                else
                {
                    throw ex;
                }
            }
        }

        async run(args)
        {
            let communicator;
            try
            {
                const [properties] = this.createTestProperties(args);
                properties.setProperty("Ice.BatchAutoFlushSize", "100");
                [communicator] = this.initialize(properties);
                await this.allTests(Test, false);
            }
            finally
            {
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
