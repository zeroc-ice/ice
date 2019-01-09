// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

import {Ice} from "ice";
import {Test} from "./generated"
import {TestHelper} from "../../../Common/TestHelper"
const test = TestHelper.test;

export class ThrowerI extends Test.Thrower
{
    shutdown(current:Ice.Current):void
    {
        current.adapter.getCommunicator().shutdown();
    }

    supportsUndeclaredExceptions(current:Ice.Current):boolean
    {
        return true;
    }

    supportsAssertException(current:Ice.Current):boolean
    {
        return false;
    }

    throwAasA(a:number, current:Ice.Current):void
    {
        throw new Test.A(a);
    }

    throwAorDasAorD(a:number, current:Ice.Current):void
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

    throwBasA(a:number, b:number, current:Ice.Current):void
    {
        this.throwBasB(a, b, current);
    }

    throwBasB(a:number, b:number, current:Ice.Current):void
    {
        throw new Test.B(a, b);
    }

    throwCasA(a:number, b:number, c:number, current:Ice.Current):void
    {
        this.throwCasC(a, b, c, current);
    }

    throwCasB(a:number, b:number, c:number, current:Ice.Current):void
    {
        this.throwCasC(a, b, c, current);
    }

    throwCasC(a:number, b:number, c:number, current:Ice.Current):void
    {
        throw new Test.C(a, b, c);
    }

    throwUndeclaredA(a:number, current:Ice.Current):void
    {
        throw new Test.A(a);
    }

    throwUndeclaredB(a:number, b:number, current:Ice.Current):void
    {
        throw new Test.B(a, b);
    }

    throwUndeclaredC(a:number, b:number, c:number, current:Ice.Current):void
    {
        throw new Test.C(a, b, c);
    }

    throwLocalException(current:Ice.Current):void
    {
        throw new Ice.TimeoutException();
    }

    throwLocalExceptionIdempotent(current:Ice.Current):void
    {
        throw new Ice.TimeoutException();
    }

    throwNonIceException(current:Ice.Current):void
    {
        throw new Error();
    }

    throwAssertException(current:Ice.Current):void
    {
        test(false);
    }

    throwMemoryLimitException(seq:Uint8Array, current:Ice.Current):Uint8Array
    {
        return new Uint8Array(1024 * 20); // 20KB is over the configured 10KB message size max.
    }

    throwAfterResponse(current:Ice.Current):void
    {
        //
        // Only relevant for AMD.
        //
    }

    throwAfterException(current:Ice.Current):void
    {
        //
        // Only relevant for AMD.
        //
        throw new Test.A();
    }
}
