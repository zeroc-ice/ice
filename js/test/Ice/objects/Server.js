//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("ice").Ice;
const Test = require("Test").Test;
require("Forward");
const TestHelper = require("TestHelper").TestHelper;
const InitialI = require("InitialI").InitialI;

class UnexpectedObjectExceptionTestI extends Test.UnexpectedObjectExceptionTest
{
    op(current)
    {
        return new Test.AlsoEmpty();
    }
}

class F2I extends Test.F2
{
    op(current)
    {
    }
}

class Server extends TestHelper
{
    async run(args)
    {
        let communicator;
        let echo;
        try
        {
            const [properties] = this.createTestProperties(args);
            properties.setProperty("Ice.Warn.Dispatch", "0");
            properties.setProperty("Ice.Warn.Connections", "0");
            [communicator] = this.initialize(properties);
            echo = await Test.EchoPrx.checkedCast(communicator.stringToProxy("__echo:" + this.getTestEndpoint()));
            const adapter = await communicator.createObjectAdapter("");
            adapter.add(new InitialI(communicator), Ice.stringToIdentity("initial"));
            adapter.add(new F2I(), Ice.stringToIdentity("F21"));
            adapter.add(new UnexpectedObjectExceptionTestI(), Ice.stringToIdentity("uoet"));
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
