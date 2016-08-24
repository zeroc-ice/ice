// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.exceptions;

import test.Ice.exceptions.AMD.Test.A;
import test.Ice.exceptions.AMD.Test.AMD_Thrower_shutdown;
import test.Ice.exceptions.AMD.Test.AMD_Thrower_supportsAssertException;
import test.Ice.exceptions.AMD.Test.AMD_Thrower_supportsUndeclaredExceptions;
import test.Ice.exceptions.AMD.Test.AMD_Thrower_throwAasA;
import test.Ice.exceptions.AMD.Test.AMD_Thrower_throwAorDasAorD;
import test.Ice.exceptions.AMD.Test.AMD_Thrower_throwAssertException;
import test.Ice.exceptions.AMD.Test.AMD_Thrower_throwMemoryLimitException;
import test.Ice.exceptions.AMD.Test.AMD_Thrower_throwBasA;
import test.Ice.exceptions.AMD.Test.AMD_Thrower_throwBasB;
import test.Ice.exceptions.AMD.Test.AMD_Thrower_throwCasA;
import test.Ice.exceptions.AMD.Test.AMD_Thrower_throwCasB;
import test.Ice.exceptions.AMD.Test.AMD_Thrower_throwCasC;
import test.Ice.exceptions.AMD.Test.AMD_Thrower_throwLocalException;
import test.Ice.exceptions.AMD.Test.AMD_Thrower_throwLocalExceptionIdempotent;
import test.Ice.exceptions.AMD.Test.AMD_Thrower_throwNonIceException;
import test.Ice.exceptions.AMD.Test.AMD_Thrower_throwUndeclaredA;
import test.Ice.exceptions.AMD.Test.AMD_Thrower_throwUndeclaredB;
import test.Ice.exceptions.AMD.Test.AMD_Thrower_throwUndeclaredC;
import test.Ice.exceptions.AMD.Test.AMD_Thrower_throwAfterResponse;
import test.Ice.exceptions.AMD.Test.AMD_Thrower_throwAfterException;
import test.Ice.exceptions.AMD.Test.B;
import test.Ice.exceptions.AMD.Test.C;
import test.Ice.exceptions.AMD.Test.D;
import test.Ice.exceptions.AMD.Test._ThrowerDisp;

public final class AMDThrowerI extends _ThrowerDisp
{
    public
    AMDThrowerI()
    {
    }

    @Override
    public void
    shutdown_async(AMD_Thrower_shutdown cb, Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        cb.ice_response();
    }

    @Override
    public void
    supportsUndeclaredExceptions_async(AMD_Thrower_supportsUndeclaredExceptions cb, Ice.Current current)
    {
        cb.ice_response(true);
    }

    @Override
    public void
    supportsAssertException_async(AMD_Thrower_supportsAssertException cb, Ice.Current current)
    {
        cb.ice_response(true);
    }

    @Override
    public void
    throwAasA_async(AMD_Thrower_throwAasA cb, int a, Ice.Current current)
        throws A
    {
        A ex = new A();
        ex.aMem = a;
        cb.ice_exception(ex);
    }

    @Override
    public void
    throwAorDasAorD_async(AMD_Thrower_throwAorDasAorD cb, int a, Ice.Current current)
        throws A,
               D
    {
        if(a > 0)
        {
            A ex = new A();
            ex.aMem = a;
            cb.ice_exception(ex);
        }
        else
        {
            D ex = new D();
            ex.dMem = a;
            cb.ice_exception(ex);
        }
    }

    @Override
    public void
    throwBasA_async(AMD_Thrower_throwBasA cb, int a, int b, Ice.Current current)
        throws A
    {
        B ex = new B();
        ex.aMem = a;
        ex.bMem = b;
        throw ex;
        //cb.ice_exception(ex);
    }

    @Override
    public void
    throwBasB_async(AMD_Thrower_throwBasB cb, int a, int b, Ice.Current current)
        throws B
    {
        B ex = new B();
        ex.aMem = a;
        ex.bMem = b;
        throw ex;
        //cb.ice_exception(ex);
    }

    @Override
    public void
    throwCasA_async(AMD_Thrower_throwCasA cb, int a, int b, int c, Ice.Current current)
        throws A
    {
        C ex = new C();
        ex.aMem = a;
        ex.bMem = b;
        ex.cMem = c;
        cb.ice_exception(ex);
    }

    @Override
    public void
    throwCasB_async(AMD_Thrower_throwCasB cb, int a, int b, int c, Ice.Current current)
        throws B
    {
        C ex = new C();
        ex.aMem = a;
        ex.bMem = b;
        ex.cMem = c;
        cb.ice_exception(ex);
    }

    @Override
    public void
    throwCasC_async(AMD_Thrower_throwCasC cb, int a, int b, int c, Ice.Current current)
        throws C
    {
        C ex = new C();
        ex.aMem = a;
        ex.bMem = b;
        ex.cMem = c;
        cb.ice_exception(ex);
    }

    @Override
    public void
    throwUndeclaredA_async(AMD_Thrower_throwUndeclaredA cb, int a, Ice.Current current)
    {
        A ex = new A();
        ex.aMem = a;
        cb.ice_exception(ex);
    }

    @Override
    public void
    throwUndeclaredB_async(AMD_Thrower_throwUndeclaredB cb, int a, int b, Ice.Current current)
    {
        B ex = new B();
        ex.aMem = a;
        ex.bMem = b;
        cb.ice_exception(ex);
    }

    @Override
    public void
    throwUndeclaredC_async(AMD_Thrower_throwUndeclaredC cb, int a, int b, int c, Ice.Current current)
    {
        C ex = new C();
        ex.aMem = a;
        ex.bMem = b;
        ex.cMem = c;
        cb.ice_exception(ex);
    }

    @Override
    public void
    throwLocalException_async(AMD_Thrower_throwLocalException cb, Ice.Current current)
    {
        cb.ice_exception(new Ice.TimeoutException());
    }

    @Override
    public void
    throwNonIceException_async(AMD_Thrower_throwNonIceException cb, Ice.Current current)
    {
        throw new RuntimeException();
    }

    @Override
    public void
    throwAssertException_async(AMD_Thrower_throwAssertException cb, Ice.Current current)
    {
        throw new java.lang.AssertionError();
    }

    @Override
    public void
    throwMemoryLimitException_async(AMD_Thrower_throwMemoryLimitException cb, byte[] seq, Ice.Current current)
    {
        cb.ice_response(new byte[1024 * 20]); // 20KB is over the configured 10KB message size max.
    }

    @Override
    public void
    throwLocalExceptionIdempotent_async(AMD_Thrower_throwLocalExceptionIdempotent cb, Ice.Current current)
    {
        cb.ice_exception(new Ice.TimeoutException());
    }

    @Override
    public void
    throwAfterResponse_async(AMD_Thrower_throwAfterResponse cb, Ice.Current current)
    {
        cb.ice_response();

        throw new RuntimeException();
    }

    @Override
    public void
    throwAfterException_async(AMD_Thrower_throwAfterException cb, Ice.Current current)
        throws A
    {
        cb.ice_exception(new A());

        throw new RuntimeException();
    }
}
