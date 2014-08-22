// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.adapterDeactivation;

import test.Ice.adapterDeactivation.Test.Cookie;

public final class ServantLocatorI implements Ice.ServantLocator
{
    public
    ServantLocatorI()
    {
        _destroyed = false;
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        test(_destroyed);
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
            test(!_destroyed);
        }

        test(current.id.category.length() == 0);
        test(current.id.name.equals("test"));

        cookie.value = new CookieI();

        return new TestI();
    }

    public void
    finished(Ice.Current current, Ice.Object servant, java.lang.Object cookie)
    {
        synchronized(this)
        {
            test(!_destroyed);
        }

        Cookie co = (Cookie)cookie;
        test(co.message().equals("blahblah"));
    }

    public synchronized void
    destroy(String category)
    {
        synchronized(this)
        {
            test(!_destroyed);

            _destroyed = true;
        }
    }

    private boolean _destroyed;
}
