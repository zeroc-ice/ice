// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
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
}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require : this.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports : this));
