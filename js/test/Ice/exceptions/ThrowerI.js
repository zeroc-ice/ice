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

    function test(value)
    {
        if(!value)
        {
            throw new Error("test failed");
        }
    }

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
            throw new Test.A(a);
        }

        throwAorDasAorD(a, current)
        {
            if(a > 0)
            {
                throw new Test.A(a);
            }
            else
            {
                throw new Test.D(a);
            }
        }

        throwBasA(a, b, current)
        {
            this.throwBasB(a, b, current);
        }

        throwBasB(a, b, current)
        {
            throw new Test.B(a, b);
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
            throw new Test.C(a, b, c);
        }

        throwUndeclaredA(a, current)
        {
            throw new Test.A(a);
        }

        throwUndeclaredB(a, b, current)
        {
            throw new Test.B(a, b);
        }

        throwUndeclaredC(a, b, c, current)
        {
            throw new Test.C(a, b, c);
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
            return new Uint8Array(1024 * 20); // 20KB is over the configured 10KB message size max.
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
