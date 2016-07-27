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
    var InitialI = require("InitialI").InitialI;
    var Client = require("../inheritance/Client");

    var Promise = Ice.Promise;

    var allTests = function(out, communicator)
    {
        return Promise.try(
            function()
            {
                return communicator.createObjectAdapter("").then(
                    function(adapter)
                    {
                        var base = communicator.stringToProxy("initial:default -p 12010");
                        adapter.add(new InitialI(adapter, base), Ice.stringToIdentity("initial"));
                        return base.ice_getConnection().then(
                            function(conn)
                            {
                                conn.setAdapter(adapter);
                                return Client.__clientAllTests__(out, communicator);
                            });
                    });
            });
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
