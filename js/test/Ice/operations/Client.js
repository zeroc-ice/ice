// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    var require = typeof(module) !== "undefined" ? module.require : function(){};

    require("Ice/Ice");
    require("Test");
    require("Twoways");
    require("Oneways");
    require("BatchOneways");

    var Ice = global.Ice;
    var Promise = Ice.Promise;
    var Twoways = global.Twoways;
    var Oneways = global.Oneways;
    var BatchOneways = global.BatchOneways;

    var allTests = function(out, communicator, Test, bidir)
    {
        var ref, base, cl, derived;

        return Promise.try(
            function()
            {
                out.write("testing twoway operations... ");
                ref = "test:default -p 12010";
                base = communicator.stringToProxy(ref);
                return Test.MyClassPrx.checkedCast(base);
            }
        ).then(
            function(prx)
            {
                cl = prx;
                return Test.MyDerivedClassPrx.checkedCast(cl);
            }
        ).then(
            function(prx)
            {
                derived = prx;
                return Twoways.run(communicator, cl, Test, bidir);
            }
        ).then(
            function()
            {
                return Twoways.run(communicator, derived, Test, bidir);
            }
        ).then(
            function()
            {
                out.writeLine("ok");
                out.write("testing oneway operations... ");
                return Oneways.run(communicator, cl, Test, bidir);
            }
        ).then(
            function()
            {
                out.writeLine("ok");
                out.write("testing batch oneway operations... ");
                return BatchOneways.run(communicator, cl, Test, bidir);
            }
        ).then(
            function()
            {
                out.writeLine("ok");
                return cl;
            });
    };

    var run = function(out, id)
    {
        return Promise.try(
            function()
            {
                //
                // We must set MessageSizeMax to an explicit value,
                // because we run tests to check whether
                // Ice.MemoryLimitException is raised as expected.
                //
                id.properties.setProperty("Ice.MessageSizeMax", "100");
                var c = Ice.initialize(id);
                return allTests(out, c, global.Test, false).then(
                    function(cl)
                    {
                        return cl.shutdown();
                    }
                ).finally(
                    function()
                    {
                        if(c)
                        {
                            return c.destroy();
                        }
                    });
            });
    };
    global.__test__ = run
    global.__clientAllTests__ = allTests;
    global.__runServer__ = true;

}(typeof (global) === "undefined" ? window : global));
