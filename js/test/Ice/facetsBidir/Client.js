// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    var Ice = require("ice").Ice;
    var Test = require("Test").Test;

    var Client = require("../facets/Client");

    var TestI = require("TestI");

    var Promise = Ice.Promise;
    var DI = TestI.DI;
    var FI = TestI.FI;
    var HI = TestI.HI;
    var EmptyI = TestI.EmptyI;

    var allTests = function(out, communicator)
    {
        var p = new Promise();
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
                    p.fail(err);
                    throw err;
                }
            }
        };
        Promise.try(
            function()
            {
                out.write("testing facet registration exceptions... ");
                return communicator.createObjectAdapter("");
            }
        ).then(
            function(adapter)
            {
                var obj = new EmptyI();
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
                var obj1 = new EmptyI();
                var obj2 = new EmptyI();
                adapter.addFacet(obj1, Ice.stringToIdentity("id1"), "f1");
                adapter.addFacet(obj2, Ice.stringToIdentity("id1"), "f2");
                var obj3 = new EmptyI();
                adapter.addFacet(obj1, Ice.stringToIdentity("id2"), "f1");
                adapter.addFacet(obj2, Ice.stringToIdentity("id2"), "f2");
                adapter.addFacet(obj3, Ice.stringToIdentity("id2"), "");
                var fm = adapter.removeAllFacets(Ice.stringToIdentity("id1"));
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

                return adapter.deactivate();
            }
        ).then(
            function(r)
            {
                return communicator.createObjectAdapter("");
            }
        ).then(
            function(adapter)
            {
                var di = new DI();
                adapter.add(di, Ice.stringToIdentity("d"));
                adapter.addFacet(di, Ice.stringToIdentity("d"), "facetABCD");
                var fi = new FI();
                adapter.addFacet(fi, Ice.stringToIdentity("d"), "facetEF");
                var hi = new HI();
                adapter.addFacet(hi, Ice.stringToIdentity("d"), "facetGH");

                var prx = Ice.ObjectPrx.uncheckedCast(communicator.stringToProxy("d:default -p 12010"));
                return prx.ice_getConnection().then(
                    function(conn)
                    {
                        conn.setAdapter(adapter);
                        return Client.__clientAllTests__(out, communicator);
                    });
            }
        ).then(
            function()
            {
                p.succeed();
            },
            function(ex)
            {
                p.fail(ex);
            });

        return p;
    };

    var run = function(out, id)
    {
        var communicator = Ice.initialize(id);
        return Promise.try(
            function()
            {
                out.writeLine("testing bidir callbacks with synchronous dispatch...");
                return allTests(out, communicator);
            }
        ).then(
            function()
            {
                return communicator.destroy();
            }
        ).then(
            function()
            {
                communicator = Ice.initialize(id);
                return Test.EchoPrx.checkedCast(communicator.stringToProxy("__echo:default -p 12010"));
            }
        ).then(
            function(prx)
            {
                return prx.shutdown();
            }
        ).finally(
            function()
            {
                return communicator.destroy();
            }
        );
    };
    exports.__test__ = run;
    exports.__runEchoServer__ = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
