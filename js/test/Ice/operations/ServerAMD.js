//
// Copyright (c) ZeroC, Inc. All rights reserved.
//


const Ice = require("ice").Ice;
const Test = require("Test").Test;
const TestHelper = require("TestHelper").TestHelper;
const AMDMyDerivedClassI = require("AMDMyDerivedClassI").AMDMyDerivedClassI;

class ServerAMD extends TestHelper
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
            adapter.add(new AMDMyDerivedClassI(echo.ice_getEndpoints()), Ice.stringToIdentity("test"));
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
exports.ServerAMD = ServerAMD;
