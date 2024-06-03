//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Test = require("Test").Test;
const ServantLocatorI = require("ServantLocatorI").ServantLocatorI;

class TestActivationI extends Test.TestActivation
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

exports.TestActivationI = TestActivationI;
