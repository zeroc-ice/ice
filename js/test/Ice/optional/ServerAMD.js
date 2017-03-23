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
    var AMDInitialI = require("AMDInitialI").AMDInitialI;


    var run = function(out, id, ready)
    {
        var communicator = Ice.initialize(id);
        var adapter;
        var echo = Test.EchoPrx.uncheckedCast(communicator.stringToProxy("__echo:default -p 12010"));
        return Ice.Promise.try(() =>
            {
                return communicator.createObjectAdapter("");
            }
        ).then(adpt =>
            {
                adapter = adpt;
                adapter.add(new AMDInitialI(), Ice.stringToIdentity("initial"));
                return echo.setConnection();
            }
        ).then(() =>
            {
                echo.ice_getCachedConnection().setAdapter(adapter);
                adapter.activate();
                ready.resolve();
                return communicator.waitForShutdown();
            }
        ).then(() =>
            {
                return echo.shutdown();
            }
        ).finally(() => communicator.destroy());
    };
    exports._serveramd = run;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
