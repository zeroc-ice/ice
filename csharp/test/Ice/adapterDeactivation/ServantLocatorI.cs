// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

public sealed class ServantLocatorI : Ice.ServantLocator
{
    public ServantLocatorI()
    {
        _deactivated = false;
    }
    
    ~ServantLocatorI()
    {
        lock(this)
        {
            test(_deactivated);
        }
    }
    
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
        }
    }
    
    public Ice.Object locate(Ice.Current current, out System.Object cookie)
    {
        lock(this)
        {
            test(!_deactivated);
        }
        
        test(current.id.category.Length == 0);
        test(current.id.name.Equals("test"));
        
        cookie = new CookieI();
        
        return new TestI();
    }
    
    public void finished(Ice.Current current, Ice.Object servant, System.Object cookie)
    {
        lock(this)
        {
            test(!_deactivated);
        }
        
        Cookie co = (Cookie) cookie;
        test(co.message().Equals("blahblah"));
    }
    
    public void deactivate(string category)
    {
        lock(this)
        {
            test(!_deactivated);
        
            _deactivated = true;
        }
    }
    
    private bool _deactivated;
}
