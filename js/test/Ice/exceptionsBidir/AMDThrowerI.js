// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

    class AMDThrowerI extends Test.Thrower
    {
        shutdown(current)
        {
            current.adapter.getCommunicator().shutdown();
            return Promise.resolve();
        }

        supportsUndeclaredExceptions(current)
        {
            return Promise.resolve(true);
        }

        supportsAssertException(current)
        {
            return Promise.resolve(false);
        }

        throwAasA(a, current)
        {
            var ex = new Test.A();
            ex.aMem = a;
            return Promise.reject(ex);
        }

        throwAorDasAorD(a, current)
        {
            var ex;
            if(a > 0)
            {
                ex = new Test.A();
                ex.aMem = a;
                return Promise.reject(ex);
            }
            else
            {
                ex = new Test.D();
                ex.dMem = a;
                return Promise.reject(ex);
            }
        }

        throwBasA(a, b, current)
        {
            return this.throwBasB(a, b, current);
        }

        throwBasB(a, b, current)
        {
            var ex = new Test.B();
            ex.aMem = a;
            ex.bMem = b;
            return Promise.reject(ex);
        }

        throwCasA(a, b, c, current)
        {
            return this.throwCasC(a, b, c, current);
        }

        throwCasB(a, b, c, current)
        {
            return this.throwCasC(a, b, c, current);
        }

        throwCasC(a, b, c, current)
        {
            var ex = new Test.C();
            ex.aMem = a;
            ex.bMem = b;
            ex.cMem = c;
            return Promise.reject(ex);
        }

        throwUndeclaredA(a, current)
        {
            var ex = new Test.A();
            ex.aMem = a;
            return Promise.reject(ex);
        }

        throwUndeclaredB(a, b, current)
        {
            var ex = new Test.B();
            ex.aMem = a;
            ex.bMem = b;
            return Promise.reject(ex);
        }

        throwUndeclaredC(a, b, c, current)
        {
            var ex = new Test.C();
            ex.aMem = a;
            ex.bMem = b;
            ex.cMem = c;
            return Promise.reject(ex);
        }

        throwLocalException(current)
        {
            return Promise.reject(new Ice.TimeoutException());
        }

        throwLocalExceptionIdempotent(current)
        {
            return Promise.reject(new Ice.TimeoutException());
        }

        throwNonIceException(current)
        {
            return Promise.reject(new Error());
        }

        throwAssertException(current)
        {
            test(false);
        }

        throwMemoryLimitException(seq, current)
        {
            return Promise.resolve(Ice.Buffer.createNative(1024 * 20)); // 20KB is over the configured 10KB message size max.
        }

        throwAfterResponse(current)
        {
            return Promise.resolve();
            //throw new Error();
        }

        throwAfterException(current)
        {
            return Promise.reject(new Test.A());
            //throw new Error();
        }
    }
    exports.AMDThrowerI = AMDThrowerI;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
