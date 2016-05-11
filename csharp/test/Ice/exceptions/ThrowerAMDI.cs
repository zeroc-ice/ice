// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using Test;

public class ThrowerI : ThrowerDisp_
{
    public ThrowerI()
    {
    }

    public override void shutdown_async(AMD_Thrower_shutdown cb, Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        cb.ice_response();
    }

    public override void supportsUndeclaredExceptions_async(AMD_Thrower_supportsUndeclaredExceptions cb, Ice.Current current)
    {
        cb.ice_response(true);
    }

    public override void supportsAssertException_async(AMD_Thrower_supportsAssertException cb, Ice.Current current)
    {
        cb.ice_response(false);
    }

    public override void throwAasA_async(AMD_Thrower_throwAasA cb, int a, Ice.Current current)
    {
        A ex = new A();
        ex.aMem = a;
        cb.ice_exception(ex);
    }

    public override void throwAorDasAorD_async(AMD_Thrower_throwAorDasAorD cb, int a, Ice.Current current)
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

    public override void throwBasA_async(AMD_Thrower_throwBasA cb, int a, int b, Ice.Current current)
    {
        B ex = new B();
        ex.aMem = a;
        ex.bMem = b;
        throw ex;
        //cb.ice_exception(ex);
    }

    public override void throwBasB_async(AMD_Thrower_throwBasB cb, int a, int b, Ice.Current current)
    {
        B ex = new B();
        ex.aMem = a;
        ex.bMem = b;
        throw ex;
        //cb.ice_exception(ex);
    }

    public override void throwCasA_async(AMD_Thrower_throwCasA cb, int a, int b, int c, Ice.Current current)
    {
        C ex = new C();
        ex.aMem = a;
        ex.bMem = b;
        ex.cMem = c;
        cb.ice_exception(ex);
    }

    public override void throwCasB_async(AMD_Thrower_throwCasB cb, int a, int b, int c, Ice.Current current)
    {
        C ex = new C();
        ex.aMem = a;
        ex.bMem = b;
        ex.cMem = c;
        cb.ice_exception(ex);
    }

    public override void throwCasC_async(AMD_Thrower_throwCasC cb, int a, int b, int c, Ice.Current current)
    {
        C ex = new C();
        ex.aMem = a;
        ex.bMem = b;
        ex.cMem = c;
        cb.ice_exception(ex);
    }

    public override void throwUndeclaredA_async(AMD_Thrower_throwUndeclaredA cb, int a, Ice.Current current)
    {
        A ex = new A();
        ex.aMem = a;
        cb.ice_exception(ex);
    }

    public override void throwUndeclaredB_async(AMD_Thrower_throwUndeclaredB cb, int a, int b, Ice.Current current)
    {
        B ex = new B();
        ex.aMem = a;
        ex.bMem = b;
        cb.ice_exception(ex);
    }

    public override void throwUndeclaredC_async(AMD_Thrower_throwUndeclaredC cb, int a, int b, int c, Ice.Current current)
    {
        C ex = new C();
        ex.aMem = a;
        ex.bMem = b;
        ex.cMem = c;
        cb.ice_exception(ex);
    }

    public override void throwLocalException_async(AMD_Thrower_throwLocalException cb, Ice.Current current)
    {
        cb.ice_exception(new Ice.TimeoutException());
    }

    public override void throwNonIceException_async(AMD_Thrower_throwNonIceException cb, Ice.Current current)
    {
        throw new Exception();
    }

    public override void throwAssertException_async(AMD_Thrower_throwAssertException cb, Ice.Current current)
    {
        Debug.Assert(false);
    }

    public override void throwMemoryLimitException_async(AMD_Thrower_throwMemoryLimitException cb, byte[] seq, 
                                                         Ice.Current current)
    {
        cb.ice_response(new byte[1024 * 20]); // 20KB is over the configured 10KB message size max.
    }

    public override void throwLocalExceptionIdempotent_async(AMD_Thrower_throwLocalExceptionIdempotent e, 
                                                             Ice.Current current)
    {
        throw new Ice.TimeoutException();
    }
    
    public override void throwAfterResponse_async(AMD_Thrower_throwAfterResponse cb, Ice.Current current)
    {
        cb.ice_response();

        throw new Exception();
    }

    public override void throwAfterException_async(AMD_Thrower_throwAfterException cb, Ice.Current current)
    {
        cb.ice_exception(new A());

        throw new Exception();
    }
}
