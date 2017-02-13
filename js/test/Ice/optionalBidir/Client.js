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
    var TestAMD = require("TestAMD").TestAMD;
    var InitialI = require("InitialI").InitialI;
    var AMDInitialI = require("AMDInitialI").AMDInitialI;
    var Client = require("../optional/Client");

    var Promise = Ice.Promise;
    var ArrayUtil = Ice.ArrayUtil;

    var allTests = function(out, communicator, amd)
    {
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
                        return Client.__clientAllTests__(out, communicator, amd ? TestAMD : Test);
                    });
            }
        );
    };

    var run = function(out, id)
    {
        var communicator = null;
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
                return Test.EchoPrx.checkedCast(base);
            }
        ).then(
            function(prx)
            {
                return prx.shutdown();
            }
        ).finally(
            function()
            {
                if(communicator)
                {
                    return communicator.destroy();
                }
            });
    };
    exports.__test__ = run;
    exports.__runEchoServer__ = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
