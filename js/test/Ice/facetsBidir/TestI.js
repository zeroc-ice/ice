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
    var Ice = require("ice").Ice;
    var Test = require("Test").Test;

    var Class = Ice.Class;

    var DI = Class(Test.D, {
        callA: function(current)
        {
            return "A";
        },
        callB: function(current)
        {
            return "B";
        },
        callC: function(current)
        {
            return "C";
        },
        callD: function(current)
        {
            return "D";
        }
    });

    exports.DI = DI;

    var EmptyI = Class(Test.Empty, {});

    exports.EmptyI = EmptyI;

    var FI = Class(Test.F, {
        callE: function(current)
        {
            return "E";
        },
        callF: function(current)
        {
            return "F";
        }
    });

    exports.FI = FI;

    var HI = Class(Test.H, {
        callG: function(current)
        {
            return "G";
        },
        callH: function(current)
        {
            return "H";
        },
        shutdown: function(current)
        {
            current.adapter.getCommunicator().shutdown();
        }
    });

    exports.HI = HI;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));

