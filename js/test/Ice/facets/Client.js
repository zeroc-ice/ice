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

        var failCB = function(){ test(false); };

        var ref, db, prx, prx2, prx3, d, df, df2, df3, ff, gf, hf;

        Promise.try(
            function()
            {
                out.write("testing stringToProxy... ");
                ref = "d:default -p 12010";
                db = communicator.stringToProxy(ref);
                test(db !== null);
                out.writeLine("ok");

                out.write("testing unchecked cast... ");
                prx = Ice.ObjectPrx.uncheckedCast(db);
                test(prx.ice_getFacet().length === 0);
                prx = Ice.ObjectPrx.uncheckedCast(db, "facetABCD");
                test(prx.ice_getFacet() == "facetABCD");
                prx2 = Ice.ObjectPrx.uncheckedCast(prx);
                test(prx2.ice_getFacet() == "facetABCD");
                prx3 = Ice.ObjectPrx.uncheckedCast(prx, "");
                test(prx3.ice_getFacet().length === 0);
                d = Test.DPrx.uncheckedCast(db);
                test(d.ice_getFacet().length === 0);
                df = Test.DPrx.uncheckedCast(db, "facetABCD");
                test(df.ice_getFacet() == "facetABCD");
                df2 = Test.DPrx.uncheckedCast(df);
                test(df2.ice_getFacet() == "facetABCD");
                df3 = Test.DPrx.uncheckedCast(df, "");
                test(df3.ice_getFacet().length === 0);
                out.writeLine("ok");
                out.write("testing checked cast... ");
                return Ice.ObjectPrx.checkedCast(db);
            }
        ).then(
            function(obj)
            {
                prx = obj;
                test(prx.ice_getFacet().length === 0);
                return Ice.ObjectPrx.checkedCast(db, "facetABCD");
            }
        ).then(
            function(obj)
            {
                prx = obj;
                test(prx.ice_getFacet() == "facetABCD");
                return Ice.ObjectPrx.checkedCast(prx);
            }
        ).then(
            function(obj)
            {
                prx2 = obj;
                test(prx2.ice_getFacet() == "facetABCD");
                return Ice.ObjectPrx.checkedCast(prx, "");
            }
        ).then(
            function(obj)
            {
                prx3 = obj;
                test(prx3.ice_getFacet().length === 0);
                return Test.DPrx.checkedCast(db);
            }
        ).then(
            function(obj)
            {
                d = obj;
                test(d.ice_getFacet().length === 0);
                return Test.DPrx.checkedCast(db, "facetABCD");
            }
        ).then(
            function(obj)
            {
                df = obj;
                test(df.ice_getFacet() == "facetABCD");
                return Test.DPrx.checkedCast(df);
            }
        ).then(
            function(obj)
            {
                df2 = obj;
                test(df2.ice_getFacet() == "facetABCD");
                return Test.DPrx.checkedCast(df, "");
            }
        ).then(
            function(obj)
            {
                df3 = obj;
                test(df3.ice_getFacet().length === 0);
                out.writeLine("ok");
                out.write("testing non-facets A, B, C, and D... ");
                return Test.DPrx.checkedCast(db);
            }
        ).then(
            function(obj)
            {
                d = obj;
                test(d !== null);
                test(d.equals(db));

                return Promise.all([d.callA(), d.callB(), d.callC(), d.callD()]);
            }
        ).then(
            function(r)
            {
                var [r1, r2, r3, r4] = r;
                test(r1 == "A");
                test(r2 == "B");
                test(r3 == "C");
                test(r4 == "D");
                out.writeLine("ok");
                out.write("testing facets A, B, C, and D... ");
                return Test.DPrx.checkedCast(d, "facetABCD");
            }
        ).then(
            function(obj)
            {
                df = obj;
                test(df !== null);

                return Promise.all([df.callA(), df.callB(), df.callC(), df.callD()]);
            }
        ).then(
            function(r)
            {
                var [r1, r2, r3, r4] = r;
                test(r1 == "A");
                test(r2 == "B");
                test(r3 == "C");
                test(r4 == "D");
                out.writeLine("ok");
                out.write("testing facets E and F... ");
                return Test.FPrx.checkedCast(d, "facetEF");
            }
        ).then(
            function(obj)
            {
                ff = obj;
                test(ff !== null);

                return Promise.all([ff.callE(), ff.callF()]);
            }
        ).then(
            function(r)
            {
                var [r1, r2] = r;
                test(r1 == "E");
                test(r2 == "F");
                out.writeLine("ok");
                out.write("testing facet G... ");
                return Test.GPrx.checkedCast(ff, "facetGH");
            }
        ).then(
            function(obj)
            {
                gf = obj;
                test(gf !== null);
                return gf.callG();
            }
        ).then(
            function(v)
            {
                test(v == "G");
                out.writeLine("ok");
                out.write("testing whether casting preserves the facet... ");
                return Test.HPrx.checkedCast(gf);
            }
        ).then(
            function(obj)
            {
                hf = obj;
                test(hf !== null);

                return Promise.all([hf.callG(), hf.callH()]);
            }
        ).then(
            function(r)
            {
                var [r1, r2] = r;
                test(r1 == "G");
                test(r2 == "H");
                out.writeLine("ok");
                return gf.shutdown();
            }
        ).then(p.resolve, p.reject);

        return p;
    };

    var run = function(out, id)
    {
        var c = Ice.initialize(id);
        return Promise.try(
            function()
            {
                return allTests(out, c);
            }
        ).finally(
            function()
            {
                return c.destroy();
            }
        );
    };
    exports._test = run;
    exports._clientAllTests = allTests;
    exports._runServer = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
