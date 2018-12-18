// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;
    const TestHelper = require("TestHelper").TestHelper;
    const ThrowerI = require("ThrowerI").ThrowerI;

    class Server extends TestHelper
    {
        async run(args)
        {
            let communicator;
            let echo;
            try
            {
                const [properties] = this.createTestProperties(args);
                properties.setProperty("Ice.MessageSizeMax", "10");
                properties.setProperty("Ice.Warn.Dispatch", "0");
                properties.setProperty("Ice.Warn.Connections", "0");
                [communicator] = this.initialize(properties);

                echo = await Test.EchoPrx.checkedCast(communicator.stringToProxy("__echo:" + this.getTestEndpoint()));
                const adapter = await communicator.createObjectAdapter("");
                adapter.add(new ThrowerI(), Ice.stringToIdentity("thrower"));
                await echo.setConnection();
                const connection = echo.ice_getCachedConnection();
                connection.setCloseCallback(con => {
                    // Re-establish connection if it fails (necessary for MemoryLimitException test)
                    echo.setConnection().then(() => echo.ice_getCachedConnection().setAdapter(adapter));
                });
                connection.setAdapter(adapter);
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
