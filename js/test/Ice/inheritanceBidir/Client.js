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
    require("InitialI");
    require("../inheritance/Client");

    var Ice = global.Ice;
    var Promise = Ice.Promise;
    var Test = global.Test;
    var InitialI = global.InitialI;

    var allTests = function(out, communicator)
    {
        return Promise.try(
            function()
            {
                return communicator.createObjectAdapter("").then(
                    function(adapter)
                    {
                        var base = communicator.stringToProxy("initial:default -p 12010");
                        adapter.add(new InitialI(adapter, base), communicator.stringToIdentity("initial"));
                        return base.ice_getConnection().then(
                            function(conn)
                            {
                                conn.setAdapter(adapter);
                                return __clientAllTests__(out, communicator);
                            });
                    });
            });
    };

    var run = function(out, id)
    {
        return Promise.try(
            function()
            {
                var communicator = Ice.initialize(id);
                out.writeLine("testing bidir callbacks with synchronous dispatch...");
                return allTests(out, communicator).then(
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
                ).then(
                    function()
                    {
                        return communicator.destroy();
                    });
            });
    };
    global.__test__ = run;
    global.__runEchoServer__ = true;
}(typeof (global) === "undefined" ? window : global));
