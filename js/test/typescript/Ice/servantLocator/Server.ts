//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import {Ice} from "ice";
import {Test} from "./generated";
import {TestHelper} from "../../../Common/TestHelper";
import {TestI} from "./TestI";
import {TestActivationI} from "./TestActivationI";
import {ServantLocatorI} from "./ServantLocatorI";

export class Server extends TestHelper
{
    async run(args:string[])
    {
        let communicator:Ice.Communicator;
        let echo:Test.EchoPrx;
        try
        {
            const [properties] = this.createTestProperties(args);
            properties.setProperty("Ice.Warn.Dispatch", "0");
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
            await adapter.waitForDeactivate();
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
