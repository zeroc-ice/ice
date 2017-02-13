// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.adapterDeactivation;

import test.Ice.adapterDeactivation.Test.Cookie;
import com.zeroc.Ice.ServantLocator;

public final class ServantLocatorI implements ServantLocator
{
    public ServantLocatorI()
    {
        _deactivated = false;
    }

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

    public ServantLocator.LocateResult locate(com.zeroc.Ice.Current current)
    {
        synchronized(this)
        {
            test(!_deactivated);
        }

        test(current.id.category.length() == 0);
        test(current.id.name.equals("test"));

        return new ServantLocator.LocateResult(new TestI(), new CookieI());
    }

    public void finished(com.zeroc.Ice.Current current, com.zeroc.Ice.Object servant, java.lang.Object cookie)
    {
        synchronized(this)
        {
            test(!_deactivated);
        }

        Cookie co = (Cookie)cookie;
        test(co.message().equals("blahblah"));
    }

    public synchronized void deactivate(String category)
    {
        synchronized(this)
        {
            test(!_deactivated);

            _deactivated = true;
        }
    }

    private boolean _deactivated;
}
