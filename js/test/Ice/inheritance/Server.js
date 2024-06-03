//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("ice").Ice;
const Test = require("Test").Test;
const TestHelper = require("TestHelper").TestHelper;
const InitialI = require("InitialI").InitialI;

class Server extends TestHelper
{
    async run(args)
    {
        let communicator;
        let echo;
        try
        {
            [communicator] = this.initialize(args);
            echo = await Test.EchoPrx.checkedCast(communicator.stringToProxy("__echo:" + this.getTestEndpoint()));
            const adapter = await communicator.createObjectAdapter("");
            const base = communicator.stringToProxy("initial:" + this.getTestEndpoint());
            adapter.add(new InitialI(adapter, base), Ice.stringToIdentity("initial"));
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
