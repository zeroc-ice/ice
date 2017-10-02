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

    class AMDThrowerI extends Test.Thrower
    {
        async shutdown(current)
        {
            current.adapter.getCommunicator().shutdown();
        }

        async supportsUndeclaredExceptions(current)
        {
            return true;
        }

        async supportsAssertException(current)
        {
            return false;
        }

        async throwAasA(a, current)
        {
            throw new Test.A(a);
        }

        async throwAorDasAorD(a, current)
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
            return this.throwBasB(a, b, current);
        }

        async throwBasB(a, b, current)
        {
            throw new Test.B(a, b);
        }

        throwCasA(a, b, c, current)
        {
            return this.throwCasC(a, b, c, current);
        }

        throwCasB(a, b, c, current)
        {
            return this.throwCasC(a, b, c, current);
        }

        async throwCasC(a, b, c, current)
        {
            throw new Test.C(a, b, c);
        }

        async throwUndeclaredA(a, current)
        {
            throw new Test.A(a);
        }

        async throwUndeclaredB(a, b, current)
        {
            throw new Test.B(a, b);
        }

        async throwUndeclaredC(a, b, c, current)
        {
            throw new Test.C(a, b, c);
        }

        async throwLocalException(current)
        {
            throw new Ice.TimeoutException();
        }

        async throwLocalExceptionIdempotent(current)
        {
            throw new Ice.TimeoutException();
        }

        async throwNonIceException(current)
        {
            throw new Error();
        }

        async throwAssertException(current)
        {
            test(false);
        }

        async throwMemoryLimitException(seq, current)
        {
            return new Uint8Array(1024 * 20); // 20KB is over the configured 10KB message size max.
        }

        async throwAfterResponse(current)
        {
            //throw new Error();
        }

        async throwAfterException(current)
        {
            throw new Test.A();
            //throw new Error();
        }
    }
    exports.AMDThrowerI = AMDThrowerI;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
