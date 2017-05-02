// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using Test;

public sealed class ThrowerI : ThrowerDisp_
{
    public ThrowerI()
    {
    }

    public override void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    public override bool supportsUndeclaredExceptions(Ice.Current current)
    {
        return true;
    }

    public override bool supportsAssertException(Ice.Current current)
    {
        return false;
    }

    public override void throwAasA(int a, Ice.Current current)
    {
        A ex = new A();
        ex.aMem = a;
        throw ex;
    }

    public override void throwAorDasAorD(int a, Ice.Current current)
    {
        if(a > 0)
        {
            A ex = new A();
            ex.aMem = a;
            throw ex;
        }
        else
        {
            D ex = new D();
            ex.dMem = a;
            throw ex;
        }
    }

    public override void throwBasA(int a, int b, Ice.Current current)
    {
        throwBasB(a, b, current);
    }

    public override void throwBasB(int a, int b, Ice.Current current)
    {
        B ex = new B();
        ex.aMem = a;
        ex.bMem = b;
        throw ex;
    }

    public override void throwCasA(int a, int b, int c, Ice.Current current)
    {
        throwCasC(a, b, c, current);
    }

    public override void throwCasB(int a, int b, int c, Ice.Current current)
    {
        throwCasC(a, b, c, current);
    }

    public override void throwCasC(int a, int b, int c, Ice.Current current)
    {
        C ex = new C();
        ex.aMem = a;
        ex.bMem = b;
        ex.cMem = c;
        throw ex;
    }

    public override void throwLocalException(Ice.Current current)
    {
        throw new Ice.TimeoutException();
    }

    public override void throwNonIceException(Ice.Current current)
    {
        throw new Exception();
    }

    public override void throwAssertException(Ice.Current current)
    {
        Debug.Assert(false);
    }

    public override byte[] throwMemoryLimitException(byte[] seq, Ice.Current current)
    {
        return new byte[1024 * 20]; // 20KB is over the configured 10KB message size max.
    }

    public override void throwLocalExceptionIdempotent(Ice.Current current)
    {
        throw new Ice.TimeoutException();
    }

    public override void throwUndeclaredA(int a, Ice.Current current)
    {
        A ex = new A();
        ex.aMem = a;
        throw ex;
    }

    public override void throwUndeclaredB(int a, int b, Ice.Current current)
    {
        B ex = new B();
        ex.aMem = a;
        ex.bMem = b;
        throw ex;
    }

    public override void throwUndeclaredC(int a, int b, int c, Ice.Current current)
    {
        C ex = new C();
        ex.aMem = a;
        ex.bMem = b;
        ex.cMem = c;
        throw ex;
    }

    public override void throwAfterResponse(Ice.Current current)
    {
        //
        // Only relevant for AMD.
        //
    }

    public override void throwAfterException(Ice.Current current)
    {
        //
        // Only relevant for AMD.
        //
        throw new A();
    }
}
