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
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;
    const InitialI = require("InitialI").InitialI;

    async function run(out, initData, ready)
    {
        let communicator;
        try
        {
            communicator = Ice.initialize(initData);
            let echo = Test.EchoPrx.uncheckedCast(communicator.stringToProxy("__echo:default -p 12010"));
            let adapter = await communicator.createObjectAdapter("");
            adapter.add(new InitialI(), Ice.stringToIdentity("initial"));
            await echo.setConnection();
            echo.ice_getCachedConnection().setAdapter(adapter);
            adapter.activate();
            ready.resolve();
            await communicator.waitForShutdown();
            await echo.shutdown();
        }
        finally
        {
            if(communicator)
            {
                await communicator.destroy();
            }
        }
    }

    exports._server = run;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
