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
    var MyDerivedClassI = require("MyDerivedClassI").MyDerivedClassI;
    var AMDMyDerivedClassI = require("AMDMyDerivedClassI").AMDMyDerivedClassI;
    var Client = require("Client.js");

    var allTests = function(out, communicator, amd)
    {
        return communicator.createObjectAdapter("").then(adapter =>
            {
                if(amd)
                {
                    adapter.add(new AMDMyDerivedClassI(), Ice.stringToIdentity("test"));
                }
                else
                {
                    adapter.add(new MyDerivedClassI(), Ice.stringToIdentity("test"));
                }
                var base = communicator.stringToProxy("test:default -p 12010");
                return base.ice_getConnection().then(conn =>
                    {
                        conn.setAdapter(adapter);
                        return Client._clientAllTests(out, communicator, Test, true);
                    });
            });
    };

    var run = function(out, id)
    {
        id.properties.setProperty("Ice.BatchAutoFlushSize", "100");
        var communicator = Ice.initialize(id);
        return Ice.Promise.try(() =>
            {
                out.writeLine("testing bidir callbacks with synchronous dispatch...");
                return allTests(out, communicator, false);
            }
        ).then(() => communicator.destroy()
        ).then(() =>
            {
                communicator = Ice.initialize(id);
                out.writeLine("testing bidir callbacks with asynchronous dispatch...");
                return allTests(out, communicator, true);
            }
        ).then(() => communicator.destroy()
        ).then(() =>
            {
                communicator = Ice.initialize(id);
                return Test.EchoPrx.checkedCast(communicator.stringToProxy("__echo:default -p 12010"));
            }
        ).then(prx => prx.shutdown()
        ).finally(() => communicator.destroy());
    };
    exports._testBidir = run;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
