//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("ice").Ice;
const Test = require("Test").Test;
const TestHelper = require("TestHelper").TestHelper;
const TestI = require("TestI").TestI;
const TestActivationI = require("TestActivationI").TestActivationI;
const ServantLocatorI = require("ServantLocatorI").ServantLocatorI;

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
            adapter.addServantLocator(new ServantLocatorI("category"), "category");
            adapter.addServantLocator(new ServantLocatorI(""), "");
            adapter.add(new TestI(), Ice.stringToIdentity("asm"));
            adapter.add(new TestActivationI(), Ice.stringToIdentity("test/activation"));
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
