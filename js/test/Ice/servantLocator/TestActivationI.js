//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { Test } from "./Test.js";
import { ServantLocatorI } from "./ServantLocatorI.js";

export class TestActivationI extends Test.TestActivation
{
    activateServantLocator(activate, current)
    {
        if(activate)
        {
            current.adapter.addServantLocator(new ServantLocatorI(""), "");
            current.adapter.addServantLocator(new ServantLocatorI("category"), "category");
        }
        else
        {
            let locator = current.adapter.removeServantLocator("");
            locator.deactivate("");
            locator = current.adapter.removeServantLocator("category");
            locator.deactivate("category");
        }
    }
}
