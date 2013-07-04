// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.exceptions;

import test.Ice.exceptions.Test.A;
import test.Ice.exceptions.Test.B;
import test.Ice.exceptions.Test.C;
import test.Ice.exceptions.Test.D;
import test.Ice.exceptions.Test._ThrowerDisp;

public final class ThrowerI extends _ThrowerDisp
{
    public
    ThrowerI()
    {
    }

    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    public boolean
    supportsUndeclaredExceptions(Ice.Current current)
    {
        return false;
    }

    public boolean
    supportsAssertException(Ice.Current current)
    {
        return true;
    }

    public void
    throwAasA(int a, Ice.Current current)
        throws A
    {
        A ex = new A();
        ex.aMem = a;
        throw ex;
    }

    public void
    throwAorDasAorD(int a, Ice.Current current)
        throws A,
               D
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

    public void
    throwBasA(int a, int b, Ice.Current current)
        throws A
    {
        throwBasB(a, b, current);
    }

    public void
    throwBasB(int a, int b, Ice.Current current)
        throws B
    {
        B ex = new B();
        ex.aMem = a;
        ex.bMem = b;
        throw ex;
    }

    public void
    throwCasA(int a, int b, int c, Ice.Current current)
        throws A
    {
        throwCasC(a, b, c, current);
    }

    public void
    throwCasB(int a, int b, int c, Ice.Current current)
        throws B
    {
        throwCasC(a, b, c, current);
    }

    public void
    throwCasC(int a, int b, int c, Ice.Current current)
        throws C
    {
        C ex = new C();
        ex.aMem = a;
        ex.bMem = b;
        ex.cMem = c;
        throw ex;
    }

    public void
    throwUndeclaredA(int a, Ice.Current current)
    {
        // Not possible in Java.
        throw new Ice.UnknownUserException();
    }

    public void
    throwUndeclaredB(int a, int b, Ice.Current current)
    {
        // Not possible in Java.
        throw new Ice.UnknownUserException();
    }

    public void
    throwUndeclaredC(int a, int b, int c, Ice.Current current)
    {
        // Not possible in Java.
        throw new Ice.UnknownUserException();
    }

    public void
    throwLocalException(Ice.Current current)
    {
        throw new Ice.TimeoutException();
    }

    public void
    throwLocalExceptionIdempotent(Ice.Current current)
    {
        throw new Ice.TimeoutException();
    }

    public void
    throwNonIceException(Ice.Current current)
    {
        throw new RuntimeException();
    }

    public void
    throwAssertException(Ice.Current current)
    {
        throw new java.lang.AssertionError();
    }

    public byte[]
    throwMemoryLimitException(byte[] seq, Ice.Current current)
    {
        return new byte[1024 * 20]; // 20KB is over the configured 10KB message size max.
    }

    public void
    throwAfterResponse(Ice.Current current)
    {
        //
        // Only relevant for AMD.
        //
    }

    public void
    throwAfterException(Ice.Current current)
        throws A
    {
        //
        // Only relevant for AMD.
        //
        throw new A();
    }
}
