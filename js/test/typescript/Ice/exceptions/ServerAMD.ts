// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import {Ice} from "ice";
import {Test} from "./generated"
import {TestHelper} from "../../../Common/TestHelper"
import {AMDThrowerI} from "./AMDThrowerI";

export class ServerAMD extends TestHelper
{
    async run(args:string[])
    {
        let communicator:Ice.Communicator;
        let echo:Test.EchoPrx;
        try
        {
            const [properties] = this.createTestProperties(args);
            properties.setProperty("Ice.MessageSizeMax", "10");
            properties.setProperty("Ice.Warn.Dispatch", "0");
            properties.setProperty("Ice.Warn.Connections", "0");
            [communicator] = this.initialize(properties);
            echo = await Test.EchoPrx.checkedCast(communicator.stringToProxy("__echo:" + this.getTestEndpoint()));
            const adapter = await communicator.createObjectAdapter("");
            adapter.add(new AMDThrowerI(), Ice.stringToIdentity("thrower"));
            await echo.setConnection();
            const connection = echo.ice_getCachedConnection();
            connection.setCloseCallback(con => {
                // Re-establish connection if it fails (necessary for MemoryLimitException test)
                echo.setConnection().then(() => echo.ice_getCachedConnection().setAdapter(adapter));
            });
            connection.setAdapter(adapter);
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
