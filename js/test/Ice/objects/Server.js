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
    var InitialI = require("InitialI").InitialI;

    class UnexpectedObjectExceptionTestI extends Test.UnexpectedObjectExceptionTest
    {
        op(current)
        {
            return new Test.AlsoEmpty();
        }
    }

    var run = function(out, id, ready)
    {
        id.properties.setProperty("Ice.Warn.Dispatch", "0");
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
                adapter.add(new InitialI(communicator), Ice.stringToIdentity("initial"));
                adapter.add(new UnexpectedObjectExceptionTestI(), Ice.stringToIdentity("uoet"));
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
    exports._server = run;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
