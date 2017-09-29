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
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;
    const TestI = require("TestI");

    const DI = TestI.DI;
    const FI = TestI.FI;
    const HI = TestI.HI;
    const EmptyI = TestI.EmptyI;

    function test(value)
    {
        if(!value)
        {
            throw new Error("test failed");
        }
    }

    async function run(out, initData, ready)
    {
        let communicator;
        try
        {
            communicator = Ice.initialize(initData);
            let echo = Test.EchoPrx.uncheckedCast(communicator.stringToProxy("__echo:default -p 12010"));

            out.write("testing facet registration exceptions... ");
            let adapter = await communicator.createObjectAdapter("");

            let obj = new EmptyI();
            adapter.add(obj, Ice.stringToIdentity("d"));
            adapter.addFacet(obj, Ice.stringToIdentity("d"), "facetABCD");
            try
            {
                adapter.addFacet(obj, Ice.stringToIdentity("d"), "facetABCD");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.AlreadyRegisteredException);
            }

            adapter.removeFacet(Ice.stringToIdentity("d"), "facetABCD");
            try
            {
                adapter.removeFacet(Ice.stringToIdentity("d"), "facetABCD");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.NotRegisteredException);
            }
            out.writeLine("ok");

            out.write("testing removeAllFacets... ");
            let obj1 = new EmptyI();
            let obj2 = new EmptyI();
            adapter.addFacet(obj1, Ice.stringToIdentity("id1"), "f1");
            adapter.addFacet(obj2, Ice.stringToIdentity("id1"), "f2");
            let obj3 = new EmptyI();
            adapter.addFacet(obj1, Ice.stringToIdentity("id2"), "f1");
            adapter.addFacet(obj2, Ice.stringToIdentity("id2"), "f2");
            adapter.addFacet(obj3, Ice.stringToIdentity("id2"), "");
            let fm = adapter.removeAllFacets(Ice.stringToIdentity("id1"));
            test(fm.size === 2);
            test(fm.get("f1") === obj1);
            test(fm.get("f2") === obj2);
            try
            {
                adapter.removeAllFacets(Ice.stringToIdentity("id1"));
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.NotRegisteredException);
            }
            fm = adapter.removeAllFacets(Ice.stringToIdentity("id2"));
            test(fm.size == 3);
            test(fm.get("f1") === obj1);
            test(fm.get("f2") === obj2);
            test(fm.get("") === obj3);
            out.writeLine("ok");

            await adapter.deactivate();
            adapter = await communicator.createObjectAdapter("");

            let di = new DI();
            adapter.add(di, Ice.stringToIdentity("d"));
            adapter.addFacet(di, Ice.stringToIdentity("d"), "facetABCD");
            let fi = new FI();
            adapter.addFacet(fi, Ice.stringToIdentity("d"), "facetEF");
            let hi = new HI();
            adapter.addFacet(hi, Ice.stringToIdentity("d"), "facetGH");
            await echo.setConnection();

            echo.ice_getCachedConnection().setAdapter(adapter);
            adapter.activate();
            ready.resolve();
            await communicator.waitForShutdown();
            await echo.shutdown();
        }
        finally
        {
            if(communicator)
            {
                await communicator.destroy();
            }
        }
    }

    exports._server = run;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
