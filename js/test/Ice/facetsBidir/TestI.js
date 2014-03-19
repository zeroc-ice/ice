// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    var Ice = global.Ice;
    var Test = global.Test;

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

    global.DI = DI;

    var EmptyI = Class(Test.Empty, {});

    global.EmptyI = EmptyI;

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

    global.FI = FI;

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

    global.HI = HI;
}(typeof (global) === "undefined" ? window : global));
