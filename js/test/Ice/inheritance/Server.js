// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
            const echo = await Test.EchoPrx.uncheckedCast(communicator.stringToProxy("__echo:default -p 12010"));
            const adapter = await communicator.createObjectAdapter("");
            const base = communicator.stringToProxy("initial:default -p 12010");
            adapter.add(new InitialI(adapter, base), Ice.stringToIdentity("initial"));
            await echo.setConnection();
            echo.ice_getCachedConnection().setAdapter(adapter);
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
