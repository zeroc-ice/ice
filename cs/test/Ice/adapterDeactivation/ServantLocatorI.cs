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

public sealed class ServantLocatorI : Ice.LocalObjectImpl, Ice.ServantLocator
{
    public ServantLocatorI()
    {
        _deactivated = false;
    }
    
    ~ServantLocatorI()
    {
        test(_deactivated);
    }
    
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
        }
    }
    
    public Ice.Object locate(Ice.Current current, out Ice.LocalObject cookie)
    {
        test(!_deactivated);
        
        test(current.id.category.Length == 0);
        test(current.id.name.Equals("test"));
        
        cookie = new CookieI();
        
        return new TestI();
    }
    
    public void finished(Ice.Current current, Ice.Object servant, Ice.LocalObject cookie)
    {
        test(!_deactivated);
        
        Cookie co = (Cookie) cookie;
        test(co.message().Equals("blahblah"));
    }
    
    public void deactivate(string category)
    {
        test(!_deactivated);
        
        _deactivated = true;
    }
    
    private bool _deactivated;
}
