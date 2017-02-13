// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.servantLocator;

import com.zeroc.Ice.ObjectNotExistException;
import com.zeroc.Ice.ServantLocator;
import com.zeroc.Ice.SocketException;
import com.zeroc.Ice.UnknownException;
import com.zeroc.Ice.UnknownLocalException;
import com.zeroc.Ice.UnknownUserException;
import com.zeroc.Ice.UserException;

import test.Ice.servantLocator.Test.Cookie;
import test.Ice.servantLocator.Test.TestImpossibleException;
import test.Ice.servantLocator.Test.TestIntfUserException;

public final class ServantLocatorI implements ServantLocator
{
    public ServantLocatorI(String category)
    {
        _category = category;
        _deactivated = false;
        _requestId = -1;
    }

    @Override
    protected synchronized void finalize()
        throws Throwable
    {
        test(_deactivated);
    }

    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    @Override
    public ServantLocator.LocateResult locate(com.zeroc.Ice.Current current)
        throws UserException
    {
        synchronized(this)
        {
            test(!_deactivated);
        }

        test(current.id.category.equals(_category) || _category.length() == 0);

        if(current.id.name.equals("unknown"))
        {
            return new ServantLocator.LocateResult();
        }

        test(current.id.name.equals("locate") || current.id.name.equals("finished"));
        if(current.id.name.equals("locate"))
        {
            exception(current);
        }

        //
        // Ensure locate() is only called once per request.
        //
        test(_requestId == -1);
        _requestId = current.requestId;

        return new ServantLocator.LocateResult(new TestI(), new CookieI());
    }

    @Override
    public void finished(com.zeroc.Ice.Current current, com.zeroc.Ice.Object servant, java.lang.Object cookie)
        throws UserException
    {
        synchronized(this)
        {
            test(!_deactivated);
        }

        //
        // Ensure finished() is only called once per request.
        //
        test(_requestId == current.requestId);
        _requestId = -1;

        test(current.id.category.equals(_category)  || _category.length() == 0);
        test(current.id.name.equals("locate") || current.id.name.equals("finished"));

        if(current.id.name.equals("finished"))
        {
            exception(current);
        }

        Cookie co = (Cookie)cookie;
        test(co.message().equals("blahblah"));
    }

    @Override
    public synchronized void deactivate(String category)
    {
        synchronized(this)
        {
            test(!_deactivated);

            _deactivated = true;
        }
    }

    private void exception(com.zeroc.Ice.Current current)
        throws UserException
    {
        if(current.operation.equals("ice_ids"))
        {
            throw new TestIntfUserException();
        }
        else if(current.operation.equals("requestFailedException"))
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
        else if(current.operation.equals("unknownExceptionWithServantException"))
        {
            throw new UnknownException("reason");
        }
        else if(current.operation.equals("impossibleException"))
        {
            throw new TestIntfUserException(); // Yes, it really is meant to be TestIntfUserException.
        }
        else if(current.operation.equals("intfUserException"))
        {
            throw new TestImpossibleException(); // Yes, it really is meant to be TestImpossibleException.
        }
        else if(current.operation.equals("asyncResponse"))
        {
            throw new TestImpossibleException();
        }
        else if(current.operation.equals("asyncException"))
        {
            throw new TestImpossibleException();
        }
    }

    private boolean _deactivated;
    private final String _category;
    private int _requestId;
}
