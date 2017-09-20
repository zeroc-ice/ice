// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    var Ice = require("ice").Ice;
    var Test = require("Test").Test;
    var ServantLocatorI = require("ServantLocatorI").ServantLocatorI;

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
                var locator;
                locator = current.adapter.removeServantLocator("");
                locator.deactivate("");
                locator = current.adapter.removeServantLocator("category");
                locator.deactivate("category");
            }
        }
    }

    exports.TestActivationI = TestActivationI;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
