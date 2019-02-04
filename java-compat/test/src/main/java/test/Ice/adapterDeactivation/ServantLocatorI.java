//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.adapterDeactivation;

import test.Ice.adapterDeactivation.Test.Cookie;

public final class ServantLocatorI implements Ice.ServantLocator
{
    final static class RouterI extends Ice._RouterDisp
    {
        public Ice.ObjectPrx getClientProxy(Ice.BooleanHolder hasRoutingTable, Ice.Current current)
        {
            hasRoutingTable.value = false;
            return null;
        }

        public Ice.ObjectPrx getServerProxy(Ice.Current current)
        {
            StringBuilder s = new StringBuilder("dummy:tcp -h localhost -p ");
            s.append(_nextPort++);
            s.append(" -t 30000");
            return current.adapter.getCommunicator().stringToProxy(s.toString());
        }

        public Ice.ObjectPrx[] addProxies(Ice.ObjectPrx[] proxies, Ice.Current current)
        {
            return null;
        }

        private int _nextPort = 23456;
    }

    public
    ServantLocatorI()
    {
        _deactivated = false;
    }

    @SuppressWarnings("deprecation")
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

        if(current.id.name.equals("router"))
        {
            return _router;
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
            test(!_deactivated);
        }

        if(current.id.name.equals("router"))
        {
            return;
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

    private boolean _deactivated;
    private Ice.Object _router = new RouterI();
}
