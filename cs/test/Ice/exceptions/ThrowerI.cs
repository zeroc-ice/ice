// **********************************************************************
//
// Copyright (c) 2003
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

using System;

public sealed class ThrowerI : Thrower_Disp
{
    public ThrowerI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }
    
    public override void shutdown(Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
    }
    
    public override bool supportsUndeclaredExceptions(Ice.Current current)
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
    
    public override void throwUndeclaredA(int a, Ice.Current current)
    {
        // Not possible in Java. TODO: can be done in C#
        throw new Ice.UnknownUserException();
    }
    
    public override void throwUndeclaredB(int a, int b, Ice.Current current)
    {
        // Not possible in Java. TODO: can be done in C#
        throw new Ice.UnknownUserException();
    }
    
    public override void throwUndeclaredC(int a, int b, int c, Ice.Current current)
    {
        // Not possible in Java. TODO: can be done in C#
        throw new Ice.UnknownUserException();
    }
    
    private Ice.ObjectAdapter _adapter;
}
