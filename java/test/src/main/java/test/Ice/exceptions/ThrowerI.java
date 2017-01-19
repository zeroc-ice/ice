// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
import test.Ice.exceptions.Test.Thrower;

public final class ThrowerI implements Thrower
{
    public ThrowerI()
    {
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    @Override
    public boolean supportsUndeclaredExceptions(com.zeroc.Ice.Current current)
    {
        return false;
    }

    @Override
    public boolean supportsAssertException(com.zeroc.Ice.Current current)
    {
        return true;
    }

    @Override
    public void throwAasA(int a, com.zeroc.Ice.Current current)
        throws A
    {
        A ex = new A();
        ex.aMem = a;
        throw ex;
    }

    @Override
    public void throwAorDasAorD(int a, com.zeroc.Ice.Current current)
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

    @Override
    public void throwBasA(int a, int b, com.zeroc.Ice.Current current)
        throws A
    {
        throwBasB(a, b, current);
    }

    @Override
    public void throwBasB(int a, int b, com.zeroc.Ice.Current current)
        throws B
    {
        B ex = new B();
        ex.aMem = a;
        ex.bMem = b;
        throw ex;
    }

    @Override
    public void throwCasA(int a, int b, int c, com.zeroc.Ice.Current current)
        throws A
    {
        throwCasC(a, b, c, current);
    }

    @Override
    public void
    throwCasB(int a, int b, int c, com.zeroc.Ice.Current current)
        throws B
    {
        throwCasC(a, b, c, current);
    }

    @Override
    public void throwCasC(int a, int b, int c, com.zeroc.Ice.Current current)
        throws C
    {
        C ex = new C();
        ex.aMem = a;
        ex.bMem = b;
        ex.cMem = c;
        throw ex;
    }

    @Override
    public void throwUndeclaredA(int a, com.zeroc.Ice.Current current)
    {
        // Not possible in Java.
        throw new com.zeroc.Ice.UnknownUserException();
    }

    @Override
    public void throwUndeclaredB(int a, int b, com.zeroc.Ice.Current current)
    {
        // Not possible in Java.
        throw new com.zeroc.Ice.UnknownUserException();
    }

    @Override
    public void throwUndeclaredC(int a, int b, int c, com.zeroc.Ice.Current current)
        throws com.zeroc.Ice.UserException
    {
        throw new C(a, b, c);
    }

    @Override
    public void throwLocalException(com.zeroc.Ice.Current current)
    {
        throw new com.zeroc.Ice.TimeoutException();
    }

    @Override
    public void throwLocalExceptionIdempotent(com.zeroc.Ice.Current current)
    {
        throw new com.zeroc.Ice.TimeoutException();
    }

    @Override
    public void throwNonIceException(com.zeroc.Ice.Current current)
    {
        throw new RuntimeException();
    }

    @Override
    public void throwAssertException(com.zeroc.Ice.Current current)
    {
        throw new java.lang.AssertionError();
    }

    @Override
    public byte[] throwMemoryLimitException(byte[] seq, com.zeroc.Ice.Current current)
    {
        return new byte[1024 * 20]; // 20KB is over the configured 10KB message size max.
    }

    @Override
    public void throwAfterResponse(com.zeroc.Ice.Current current)
    {
        //
        // Only relevant for AMD.
        //
    }

    @Override
    public void throwAfterException(com.zeroc.Ice.Current current)
        throws A
    {
        //
        // Only relevant for AMD.
        //
        throw new A();
    }
}
