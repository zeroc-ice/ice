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

public final class ServantLocatorI extends Ice.LocalObjectImpl implements Ice.ServantLocator
{
    public
    ServantLocatorI()
    {
        _deactivated = false;
    }

    protected void
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
        test(!_deactivated);

        test(current.id.category.length() == 0);
        test(current.id.name.equals("test"));

        cookie.value = new CookieI();

        return new TestI();
    }

    public void
    finished(Ice.Current current, Ice.Object servant, Ice.LocalObject cookie)
    {
        test(!_deactivated);

        Cookie co = (Cookie)cookie;
        test(co.message().equals("blahblah"));
    }

    public void
    deactivate()
    {
        test(!_deactivated);

        _deactivated = true;
    }

    private boolean _deactivated;
}
