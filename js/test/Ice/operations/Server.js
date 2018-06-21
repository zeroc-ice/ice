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
    const MyDerivedClassI = require("MyDerivedClassI").MyDerivedClassI;

    async function run(out, initData, ready)
    {
        initData.properties.setProperty("Ice.BatchAutoFlushSize", "100");
        let communicator;
        try
        {
            let echo;
            try
            {
                communicator = Ice.initialize(initData);
                echo = await Test.EchoPrx.checkedCast(communicator.stringToProxy("__echo:default -p 12010"));
                const adapter = await communicator.createObjectAdapter("");
                adapter.add(new MyDerivedClassI(echo.ice_getEndpoints()), Ice.stringToIdentity("test"));
                await echo.setConnection();
                echo.ice_getCachedConnection().setAdapter(adapter);
                ready.resolve();
                await communicator.waitForShutdown();
            }
            catch(ex)
            {
                ready.reject(ex);
            }
            finally
            {
                if(echo)
                {
                    await echo.shutdown();
                }
            }
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
}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require : this.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports : this));
