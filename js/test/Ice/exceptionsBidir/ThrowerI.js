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

    var test = function(b)
    {
        if(!b)
        {
            throw new Error("test failed");
        }
    };

    var ThrowerI = Class(Test.Thrower, {
        shutdown: function(current)
        {
            current.adapter.getCommunicator().shutdown();
        },

        supportsUndeclaredExceptions: function(current)
        {
            return true;
        },

        supportsAssertException: function(current)
        {
            return false;
        },

        throwAasA: function(a, current)
        {
            var ex = new Test.A();
            ex.aMem = a;
            throw ex;
        },

        throwAorDasAorD: function(a, current)
        {
            var ex;
            if(a > 0)
            {
                ex = new Test.A();
                ex.aMem = a;
                throw ex;
            }
            else
            {
                ex = new Test.D();
                ex.dMem = a;
                throw ex;
            }
        },

        throwBasA: function(a, b, current)
        {
            this.throwBasB(a, b, current);
        },

        throwBasB: function(a, b, current)
        {
            var ex = new Test.B();
            ex.aMem = a;
            ex.bMem = b;
            throw ex;
        },

        throwCasA: function(a, b, c, current)
        {
            this.throwCasC(a, b, c, current);
        },

        throwCasB: function(a, b, c, current)
        {
            this.throwCasC(a, b, c, current);
        },

        throwCasC: function(a, b, c, current)
        {
            var ex = new Test.C();
            ex.aMem = a;
            ex.bMem = b;
            ex.cMem = c;
            throw ex;
        },

        throwUndeclaredA: function(a, current)
        {
            var ex = new Test.A();
            ex.aMem = a;
            throw ex;
        },

        throwUndeclaredB: function(a, b, current)
        {
            var ex = new Test.B();
            ex.aMem = a;
            ex.bMem = b;
            throw ex;
        },

        throwUndeclaredC: function(a, b, c, current)
        {
            var ex = new Test.C();
            ex.aMem = a;
            ex.bMem = b;
            ex.cMem = c;
            throw ex;
        },

        throwLocalException: function(current)
        {
            throw new Ice.TimeoutException();
        },

        throwLocalExceptionIdempotent: function(current)
        {
            throw new Ice.TimeoutException();
        },

        throwNonIceException: function(current)
        {
            throw new Error();
        },

        throwAssertException: function(current)
        {
            test(false);
        },

        throwMemoryLimitException: function(seq, current)
        {
            return Ice.Buffer.createNative(1024 * 20); // 20KB is over the configured 10KB message size max.
        },

        throwAfterResponse: function(current)
        {
            //
            // Only relevant for AMD.
            //
        },

        throwAfterException: function(current)
        {
            //
            // Only relevant for AMD.
            //
            throw new Test.A();
        },
    });

    exports.ThrowerI = ThrowerI;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
