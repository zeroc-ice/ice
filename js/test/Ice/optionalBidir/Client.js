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
    var Ice = global.Ice;

    var Promise = Ice.Promise;
    var ArrayUtil = Ice.ArrayUtil;

    require("Test");
    require("TestAMD");

    require("InitialI");
    require("AMDInitialI");
    require("../optional/Client");
    var InitialI = global.InitialI;
    var AMDInitialI = global.AMDInitialI;

    var allTests = function(out, communicator, amd)
    {
        var Test = amd ? global.TestAMD : global.Test;
        var base;
        return Promise.try(
            function()
            {
                base = communicator.stringToProxy("initial:default -p 12010");
                return communicator.createObjectAdapter("");
            }
        ).then(
            function(adapter)
            {
                if(amd)
                {
                    adapter.add(new AMDInitialI(), communicator.stringToIdentity("initial"));
                }
                else
                {
                    adapter.add(new InitialI(), communicator.stringToIdentity("initial"));
                }
                return base.ice_getConnection().then(
                    function(conn)
                    {
                        conn.setAdapter(adapter);
                        return __clientAllTests__(out, communicator, Test);
                    });
            }
        );
    };

    var run = function(out, id)
    {
         var communicator;
        return Promise.try(
            function()
            {
                communicator = Ice.initialize(id);
                out.writeLine("testing bidir callbacks with synchronous dispatch...");
                return allTests(out, communicator, false);
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
                out.writeLine("testing bidir callbacks with asynchronous dispatch...");
                return allTests(out, communicator, true);
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
                var base = communicator.stringToProxy("__echo:default -p 12010");
                return global.Test.EchoPrx.checkedCast(base);
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
            });
    };
    global.__test__ = run;
    global.__runEchoServer__ = true;
}(typeof (global) === "undefined" ? window : global));

