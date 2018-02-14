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

    async function allTests(out, communicator)
    {
        function test(value)
        {
            if(!value)
            {
                throw new Error("test failed");
            }
        }

        out.write("testing stringToProxy... ");
        let ref = "d:default -p 12010";
        let db = communicator.stringToProxy(ref);
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
        def3 = await Test.DPrx.checkedCast(df, "");
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
        let ff = await Test.FPrx.checkedCast(d, "facetEF");
        test(await ff.callE() == "E");
        test(await ff.callF() == "F");
        out.writeLine("ok");

        out.write("testing facet G... ");
        let gf = await Test.GPrx.checkedCast(ff, "facetGH");
        test(gf !== null);
        test(await gf.callG() == "G");
        out.writeLine("ok");

        out.write("testing whether casting preserves the facet... ");
        let hf = await Test.HPrx.checkedCast(gf);
        test(hf !== null);
        test(await hf.callG() == "G");
        test(await hf.callH() == "H");
        out.writeLine("ok");

        await gf.shutdown();
    }

    async function run(out, initData)
    {
        let communicator;
        try
        {
            communicator = Ice.initialize(initData);
            await allTests(out, communicator);
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
