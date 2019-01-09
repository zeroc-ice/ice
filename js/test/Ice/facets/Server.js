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
    const TestI = require("TestI");
    const test = TestHelper.test;

    const DI = TestI.DI;
    const FI = TestI.FI;
    const HI = TestI.HI;
    const EmptyI = TestI.EmptyI;

    class Server extends TestHelper
    {
        async run(args)
        {
            let communicator;
            let echo;
            try
            {
                [communicator] = this.initialize(args);
                const out = this.getWriter();
                echo = await Test.EchoPrx.checkedCast(communicator.stringToProxy("__echo:" + this.getTestEndpoint()));

                out.write("testing facet registration exceptions... ");
                let adapter = await communicator.createObjectAdapter("");

                const obj = new EmptyI();
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
                const obj1 = new EmptyI();
                const obj2 = new EmptyI();
                adapter.addFacet(obj1, Ice.stringToIdentity("id1"), "f1");
                adapter.addFacet(obj2, Ice.stringToIdentity("id1"), "f2");
                const obj3 = new EmptyI();
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

                const di = new DI();
                adapter.add(di, Ice.stringToIdentity("d"));
                adapter.addFacet(di, Ice.stringToIdentity("d"), "facetABCD");
                const fi = new FI();
                adapter.addFacet(fi, Ice.stringToIdentity("d"), "facetEF");
                const hi = new HI();
                adapter.addFacet(hi, Ice.stringToIdentity("d"), "facetGH");
                await echo.setConnection();
                echo.ice_getCachedConnection().setAdapter(adapter);
                this.serverReady();
                await communicator.waitForShutdown();
            }
            finally
            {
                if(echo)
                {
                    await echo.shutdown();
                }

                if(communicator)
                {
                    await communicator.destroy();
                }
            }
        }
    }
    exports.Server = Server;
}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self.Ice._require : window.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self : window));
