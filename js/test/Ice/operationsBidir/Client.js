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
    var MyDerivedClassI = require("MyDerivedClassI").MyDerivedClassI;
    var AMDMyDerivedClassI = require("AMDMyDerivedClassI").AMDMyDerivedClassI;
    var Client = require("../operations/Client.js");

    var Promise = Ice.Promise;

    var allTests = function(out, communicator, amd)
    {
        return communicator.createObjectAdapter("").then(
            function(adapter)
            {
                if(amd)
                {
                    adapter.add(new AMDMyDerivedClassI(), communicator.stringToIdentity("test"));
                }
                else
                {
                    adapter.add(new MyDerivedClassI(), communicator.stringToIdentity("test"));
                }
                var base = communicator.stringToProxy("test:default -p 12010");
                return base.ice_getConnection().then(
                    function(conn)
                    {
                        conn.setAdapter(adapter);
                        return Client.__clientAllTests__(out, communicator, amd ? TestAMD : Test, true);
                    });
            });
    };

    var run = function(out, id)
    {
        id.properties.setProperty("Ice.BatchAutoFlushSize", "100");
        var communicator = Ice.initialize(id);
        return Promise.try(
            function()
            {
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
