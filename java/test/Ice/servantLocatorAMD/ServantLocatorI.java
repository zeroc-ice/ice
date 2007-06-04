// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;
import Ice.*;

public final class ServantLocatorI extends Ice.LocalObjectImpl implements Ice.ServantLocator
{
    public
    ServantLocatorI(String category)
    {
        _category = category;
        _deactivated = false;
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        test(_deactivated);
    }

    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public Ice.Object
    locate(Ice.Current current, Ice.LocalObjectHolder cookie)
    {
        synchronized(this)
        {
            test(!_deactivated);
        }

        test(current.id.category.equals(_category) || _category.length() == 0);
        
        if(current.id.name.equals("unknown"))
        {
            return null;
        }

        test(current.id.name.equals("locate") || current.id.name.equals("finished"));
        if(current.id.name.equals("locate"))
        {
            exception(current);
        }

        cookie.value = new CookieI();

        return new TestI();
    }

    public void
    finished(Ice.Current current, Ice.Object servant, java.lang.Object cookie)
    {
        synchronized(this)
        {
            test(!_deactivated);
        }

        test(current.id.category.equals(_category)  || _category.length() == 0);
        test(current.id.name.equals("locate") || current.id.name.equals("finished"));
        
        if(current.id.name.equals("finished"))
        {
            exception(current);
        }
        
        Cookie co = (Cookie)cookie;
        test(co.message().equals("blahblah"));
    }

    public synchronized void
    deactivate(String category)
    {
        synchronized(this)
        {
            test(!_deactivated);

            _deactivated = true;
        }
    }

    private void
    exception(Ice.Current current)
    {
        if(current.operation.equals("requestFailedException"))
        {
            throw new ObjectNotExistException();
        }
        else if(current.operation.equals("unknownUserException"))
        {
            throw new UnknownUserException("reason");
        }
        else if(current.operation.equals("unknownLocalException"))
        {
            throw new UnknownLocalException("reason");
        }
        else if(current.operation.equals("unknownException"))
        {
            throw new UnknownException("reason");
        }
        //
        // User exceptions are checked exceptions in Java, so it's not
        // possible to throw it from the servant locator.
        // 
//      else if(current.operation.equals("userException"))
//      {
//          throw new TestIntfUserException();
//      }
        else if(current.operation.equals("localException"))
        {
            throw new SocketException(0);
        }
        else if(current.operation.equals("javaException"))
        {
            throw new java.lang.RuntimeException("message");
        }
    }

    private boolean _deactivated;
    private final String _category;
}
