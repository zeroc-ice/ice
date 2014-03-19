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

    require("Test");
    require("TestAMD");
    require("../exceptions/Client");

    require("ThrowerI");
    var ThrowerI = global.ThrowerI;
    require("AMDThrowerI");
    var AMDThrowerI = global.AMDThrowerI;


    var allTests = function(out, communicator, amd)
    {
        return Promise.try(
            function()
            {
                return communicator.createObjectAdapter("").then(
                    function(adapter)
                    {
                        if(amd)
                        {
                            adapter.add(new AMDThrowerI(), communicator.stringToIdentity("thrower"));
                        }
                        else
                        {
                            adapter.add(new ThrowerI(), communicator.stringToIdentity("thrower"));
                        }
                        var base = communicator.stringToProxy("thrower:default -p 12010");
                        return base.ice_getConnection().then(
                            function(conn)
                            {
                                conn.setAdapter(adapter);
                                return __clientAllTests__(out, communicator, amd ? global.TestAMD : global.Test);
                            });
                    });
            });
    };

    var run = function(out, id)
    {
       return Promise.try(
            function()
            {
                id.properties.setProperty("Ice.MessageSizeMax", "10");
                id.properties.setProperty("Ice.Warn.Dispatch", "0");
                var communicator = Ice.initialize(id);
                out.writeLine("testing bidir callbacks with synchronous dispatch...");
                return allTests(out, communicator, false).then(
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
                        var ref = "__echo:default -p 12010";
                        var base = communicator.stringToProxy(ref);
                        return global.Test.EchoPrx.checkedCast(base);
                    }
                ).then(
                    function(prx)
                    {
                        return prx.shutdown();
                    }
                ).then(
                    function()
                    {
                        return communicator.destroy();
                    });
            });
        return p;
    };
    global.__test__ = run;
    global.__runEchoServer__ = true;
}(typeof (global) === "undefined" ? window : global));
