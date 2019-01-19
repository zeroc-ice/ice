//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

(function(module, require, exports)
{
    const Ice = require("ice").Ice;
    const TestHelper = require("TestHelper").TestHelper;
    const Test = require("Test").Test;
    const test = TestHelper.test;

    class Client extends TestHelper
    {
        async allTests()
        {
            const communicator = this.communicator();
            const out = this.getWriter();

            out.write("testing stringToProxy... ");
            const ref = "d:" + this.getTestEndpoint();
            const db = communicator.stringToProxy(ref);
            test(db !== null);
            out.writeLine("ok");

            out.write("testing unchecked cast... ");
            let prx = Ice.ObjectPrx.uncheckedCast(db);
            test(prx.ice_getFacet().length === 0);
            prx = Ice.ObjectPrx.uncheckedCast(db, "facetABCD");
            test(prx.ice_getFacet() == "facetABCD");
            let prx2 = Ice.ObjectPrx.uncheckedCast(prx);
            test(prx2.ice_getFacet() == "facetABCD");
            let prx3 = Ice.ObjectPrx.uncheckedCast(prx, "");
            test(prx3.ice_getFacet().length === 0);
            let d = Test.DPrx.uncheckedCast(db);
            test(d.ice_getFacet().length === 0);
            let df = Test.DPrx.uncheckedCast(db, "facetABCD");
            test(df.ice_getFacet() == "facetABCD");
            let df2 = Test.DPrx.uncheckedCast(df);
            test(df2.ice_getFacet() == "facetABCD");
            let df3 = Test.DPrx.uncheckedCast(df, "");
            test(df3.ice_getFacet().length === 0);
            out.writeLine("ok");

            out.write("testing checked cast... ");
            prx = await Ice.ObjectPrx.checkedCast(db);
            test(prx.ice_getFacet().length === 0);
            prx = await Ice.ObjectPrx.checkedCast(db, "facetABCD");
            test(prx.ice_getFacet() == "facetABCD");
            prx2 = await Ice.ObjectPrx.checkedCast(prx);
            test(prx2.ice_getFacet() == "facetABCD");
            prx3 = await Ice.ObjectPrx.checkedCast(prx, "");
            test(prx3.ice_getFacet().length === 0);
            d = await Test.DPrx.checkedCast(db);
            test(d.ice_getFacet().length === 0);
            df = await Test.DPrx.checkedCast(db, "facetABCD");
            test(df.ice_getFacet() == "facetABCD");
            df2 = await Test.DPrx.checkedCast(df);
            test(df2.ice_getFacet() == "facetABCD");
            df3 = await Test.DPrx.checkedCast(df, "");
            test(df3.ice_getFacet().length === 0);
            out.writeLine("ok");

            out.write("testing non-facets A, B, C, and D... ");
            d = await Test.DPrx.checkedCast(db);
            test(d !== null);
            test(d.equals(db));
            test(await d.callA() == "A");
            test(await d.callB() == "B");
            test(await d.callC() == "C");
            test(await d.callD() == "D");
            out.writeLine("ok");

            out.write("testing facets A, B, C, and D... ");
            df = await Test.DPrx.checkedCast(d, "facetABCD");
            test(df !== null);
            test(await df.callA() == "A");
            test(await df.callB() == "B");
            test(await df.callC() == "C");
            test(await df.callD() == "D");
            out.writeLine("ok");

            out.write("testing facets E and F... ");
            const ff = await Test.FPrx.checkedCast(d, "facetEF");
            test(await ff.callE() == "E");
            test(await ff.callF() == "F");
            out.writeLine("ok");

            out.write("testing facet G... ");
            const gf = await Test.GPrx.checkedCast(ff, "facetGH");
            test(gf !== null);
            test(await gf.callG() == "G");
            out.writeLine("ok");

            out.write("testing whether casting preserves the facet... ");
            const hf = await Test.HPrx.checkedCast(gf);
            test(hf !== null);
            test(await hf.callG() == "G");
            test(await hf.callH() == "H");
            out.writeLine("ok");

            await gf.shutdown();
        }

        async run(args)
        {
            let communicator;
            try
            {
                [communicator] = this.initialize(args);
                await this.allTests();
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
