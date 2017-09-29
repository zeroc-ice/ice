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
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;

    class DI extends Test.D
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
    }

    class EmptyI extends Test.Empty
    {
    }

    class FI extends Test.F
    {
        callE(current)
        {
            return "E";
        }

        callF(current)
        {
            return "F";
        }
    }

    class HI extends Test.H
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
    }

    exports.DI = DI;
    exports.EmptyI = EmptyI;
    exports.FI = FI;
    exports.HI = HI;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
