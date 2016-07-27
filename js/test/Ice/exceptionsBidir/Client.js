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
    var TestAMD = require("TestAMD").TestAMD;

    var Promise = Ice.Promise;

    var Client = require("../exceptions/Client");

    var ThrowerI = require("ThrowerI").ThrowerI;
    var AMDThrowerI = require("AMDThrowerI").AMDThrowerI;

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
                            adapter.add(new AMDThrowerI(), Ice.stringToIdentity("thrower"));
                        }
                        else
                        {
                            adapter.add(new ThrowerI(), Ice.stringToIdentity("thrower"));
                        }
                        var base = communicator.stringToProxy("thrower:default -p 12010");
                        return base.ice_getConnection().then(
                            function(conn)
                            {
                                conn.setAdapter(adapter);
                                return Client.__clientAllTests__(out, communicator, amd ? TestAMD : Test, true);
                            });
                    });
            });
    };

    var run = function(out, id)
    {
        id.properties.setProperty("Ice.MessageSizeMax", "10");
        id.properties.setProperty("Ice.Warn.Dispatch", "0");
        id.properties.setProperty("Ice.Warn.Connections", "0");
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
                var ref = "__echo:default -p 12010";
                var base = communicator.stringToProxy(ref);
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
                return communicator.destroy();
            }
        );
    };
    exports.__test__ = run;
    exports.__runEchoServer__ = true;
    exports.__runEchoServerOptions__ = ["Ice.Warn.Dispatch=0", "Ice.Warn.Connections=0"];
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
