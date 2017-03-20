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

    exports.DI = class extends Test.D
    {
        callA(current)
        {
            return "A";
        }

        callB(current)
        {
            return "B";
        }

        callC(current)
        {
            return "C";
        }

        callD(current)
        {
            return "D";
        }
    };

    exports.EmptyI = class extends Test.Empty
    {
    };

    exports.FI = class extends Test.F
    {
        callE(current)
        {
            return "E";
        }

        callF(current)
        {
            return "F";
        }
    };

    exports.HI = class extends Test.H
    {
        callG(current)
        {
            return "G";
        }

        callH(current)
        {
            return "H";
        }

        shutdown(current)
        {
            current.adapter.getCommunicator().shutdown();
        }
    };
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
