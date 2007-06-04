// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Test;
using Ice;

public sealed class ServantLocatorI : Ice.ServantLocator
{
    public ServantLocatorI(String category)
    {
        _category = category;
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
        
        test(current.id.category.Equals(_category) || _category.Length == 0);
        
        if(current.id.name.Equals("unknown"))
        {
            cookie = null;
            return null;
        }

        test(current.id.name.Equals("locate") || current.id.name.Equals("finished"));
        if(current.id.name.Equals("locate"))
        {
            exception(current);
        }

        cookie = new CookieI();

        return new TestI();
    }
    
    public void finished(Ice.Current current, Ice.Object servant, System.Object cookie)
    {
        lock(this)
        {
            test(!_deactivated);
        }
        
        test(current.id.category.Equals(_category)  || _category.Length == 0);
        test(current.id.name.Equals("locate") || current.id.name.Equals("finished"));
        
        if(current.id.name.Equals("finished"))
        {
            exception(current);
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
    
    private void exception(Ice.Current current)
    {
        if(current.operation.Equals("requestFailedException"))
        {
            throw new ObjectNotExistException();
        }
        else if(current.operation.Equals("unknownUserException"))
        {
            UnknownUserException ex = new UnknownUserException();
            ex.unknown = "reason";
            throw ex;
        }
        else if(current.operation.Equals("unknownLocalException"))
        {
            UnknownLocalException ex = new UnknownLocalException();
            ex.unknown = "reason";
            throw ex;
        }
        else if(current.operation.Equals("unknownException"))
        {
            UnknownException ex = new UnknownException();
            ex.unknown = "reason";
            throw ex;
        }
        else if(current.operation.Equals("userException"))
        {
            throw new TestIntfUserException();
        }
        else if(current.operation.Equals("localException"))
        {
            SocketException ex = new SocketException();
            ex.error = 0;
            throw ex;
        }
        else if(current.operation.Equals("csException"))
        {
            throw new System.Exception("message");
        }
    }

    private bool _deactivated;
    private string _category;
}
