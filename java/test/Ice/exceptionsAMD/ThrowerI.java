// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

// Ice version 0.0.1

public final class ThrowerI extends _ThrowerDisp
{
    public
    ThrowerI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    public void
    shutdown_async(AMD_Thrower_shutdown cb, Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
	cb.ice_response();
    }

    public void
    supportsUndeclaredExceptions_async(AMD_Thrower_supportsUndeclaredExceptions cb, Ice.Current current)
    {
        cb.ice_response(true);
    }

    public void
    throwAasA_async(AMD_Thrower_throwAasA cb, int a, Ice.Current current)
        throws A
    {
        A ex = new A();
        ex.aMem = a;
        cb.ice_exception(ex);
    }

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

    public void
    throwLocalException_async(AMD_Thrower_throwLocalException cb, Ice.Current current)
    {
        cb.ice_exception(new Ice.TimeoutException());
    }

    public void
    throwNonIceException_async(AMD_Thrower_throwNonIceException cb, Ice.Current current)
    {
        throw new RuntimeException();
    }

    public void
    throwUndeclaredA_async(AMD_Thrower_throwUndeclaredA cb, int a, Ice.Current current)
    {
        A ex = new A();
        ex.aMem = a;
        cb.ice_exception(ex);
    }

    public void
    throwUndeclaredB_async(AMD_Thrower_throwUndeclaredB cb, int a, int b, Ice.Current current)
    {
	B ex = new B();
	ex.aMem = a;
	ex.bMem = b;
	cb.ice_exception(ex);
    }

    public void
    throwUndeclaredC_async(AMD_Thrower_throwUndeclaredC cb, int a, int b, int c, Ice.Current current)
    {
        C ex = new C();
        ex.aMem = a;
        ex.bMem = b;
        ex.cMem = c;
        cb.ice_exception(ex);
    }

    private Ice.ObjectAdapter _adapter;
}
