// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import {Ice} from "ice";
import {Test} from "./generated";
import {TestHelper} from "../../../Common/TestHelper";
import {AMDMyDerivedClassI} from "./AMDMyDerivedClassI";

export class ServerAMD extends TestHelper
{
    async run(args:string[])
    {
        let communicator:Ice.Communicator;
        let echo:Test.EchoPrx;
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
