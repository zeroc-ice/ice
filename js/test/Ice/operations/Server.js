// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

(function(module, require, exports)
{
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;
    const TestHelper = require("TestHelper").TestHelper;
    const MyDerivedClassI = require("MyDerivedClassI").MyDerivedClassI;

    class Server extends TestHelper
    {
        async run(args)
        {
            let communicator;
            let echo;
            try
            {
                const [properties] = this.createTestProperties(args);
                properties.setProperty("Ice.BatchAutoFlushSize", "100");
                [communicator] = this.initialize(properties);
                echo = await Test.EchoPrx.checkedCast(communicator.stringToProxy("__echo:" + this.getTestEndpoint()));
                const adapter = await communicator.createObjectAdapter("");
                adapter.add(new MyDerivedClassI(echo.ice_getEndpoints()), Ice.stringToIdentity("test"));
                await echo.setConnection();
                echo.ice_getCachedConnection().setAdapter(adapter);
                this.serverReady();
                await communicator.waitForShutdown();
            }
            finally
            {
                if(echo)
                {
                    await echo.shutdown();
                }

                if(communicator)
                {
                    await communicator.destroy();
                }
            }
        }
    }
    exports.Server = Server;
}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self.Ice._require : window.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self : window));
