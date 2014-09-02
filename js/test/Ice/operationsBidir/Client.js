// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    var Ice = require("icejs").Ice;
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
        return Promise.try(
            function()
            {
                //
                // We must set MessageSizeMax to an explicit value,
                // because we run tests to check whether
                // Ice.MemoryLimitException is raised as expected.
                //
                id.properties.setProperty("Ice.MessageSizeMax", "100");
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
    exports.__test__ = run;
    exports.__runEchoServer__ = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : window.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : window));
