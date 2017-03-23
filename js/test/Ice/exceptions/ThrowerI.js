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

    var test = function(b)
    {
        if(!b)
        {
            throw new Error("test failed");
        }
    };

    class ThrowerI extends Test.Thrower
    {
        shutdown(current)
        {
            current.adapter.getCommunicator().shutdown();
        }

        supportsUndeclaredExceptions(current)
        {
            return true;
        }

        supportsAssertException(current)
        {
            return false;
        }

        throwAasA(a, current)
        {
            var ex = new Test.A();
            ex.aMem = a;
            throw ex;
        }

        throwAorDasAorD(a, current)
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
        }

        throwBasA(a, b, current)
        {
            this.throwBasB(a, b, current);
        }

        throwBasB(a, b, current)
        {
            var ex = new Test.B();
            ex.aMem = a;
            ex.bMem = b;
            throw ex;
        }

        throwCasA(a, b, c, current)
        {
            this.throwCasC(a, b, c, current);
        }

        throwCasB(a, b, c, current)
        {
            this.throwCasC(a, b, c, current);
        }

        throwCasC(a, b, c, current)
        {
            var ex = new Test.C();
            ex.aMem = a;
            ex.bMem = b;
            ex.cMem = c;
            throw ex;
        }

        throwUndeclaredA(a, current)
        {
            var ex = new Test.A();
            ex.aMem = a;
            throw ex;
        }

        throwUndeclaredB(a, b, current)
        {
            var ex = new Test.B();
            ex.aMem = a;
            ex.bMem = b;
            throw ex;
        }

        throwUndeclaredC(a, b, c, current)
        {
            var ex = new Test.C();
            ex.aMem = a;
            ex.bMem = b;
            ex.cMem = c;
            throw ex;
        }

        throwLocalException(current)
        {
            throw new Ice.TimeoutException();
        }

        throwLocalExceptionIdempotent(current)
        {
            throw new Ice.TimeoutException();
        }

        throwNonIceException(current)
        {
            throw new Error();
        }

        throwAssertException(current)
        {
            test(false);
        }

        throwMemoryLimitException(seq, current)
        {
            return Ice.Buffer.createNative(1024 * 20); // 20KB is over the configured 10KB message size max.
        }

        throwAfterResponse(current)
        {
            //
            // Only relevant for AMD.
            //
        }

        throwAfterException(current)
        {
            //
            // Only relevant for AMD.
            //
            throw new Test.A();
        }
    }

    exports.ThrowerI = ThrowerI;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
