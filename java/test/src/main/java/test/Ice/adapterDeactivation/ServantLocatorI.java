//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.adapterDeactivation;

import com.zeroc.Ice.ServantLocator;

public final class ServantLocatorI implements ServantLocator
{
    final static class RouterI implements com.zeroc.Ice.Router
    {
        public com.zeroc.Ice.Router.GetClientProxyResult getClientProxy(com.zeroc.Ice.Current current)
        {
            return new com.zeroc.Ice.Router.GetClientProxyResult();
        }

        public com.zeroc.Ice.ObjectPrx getServerProxy(com.zeroc.Ice.Current current)
        {
            StringBuilder s = new StringBuilder("dummy:tcp -h localhost -p ");
            s.append(_nextPort++);
            s.append(" -t 30000");
            return current.adapter.getCommunicator().stringToProxy(s.toString());
        }

        public com.zeroc.Ice.ObjectPrx[] addProxies(com.zeroc.Ice.ObjectPrx[] proxies, com.zeroc.Ice.Current current)
        {
            return null;
        }

        private int _nextPort = 23456;
    }

    public ServantLocatorI()
    {
        _deactivated = false;
    }

    @SuppressWarnings( "deprecation" )
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

        if(current.id.name.equals("router"))
        {
            return new ServantLocator.LocateResult(_router, null);
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

        if(current.id.name.equals("router"))
        {
            return;
        }

        CookieI co = (CookieI)cookie;
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
    private com.zeroc.Ice.Object _router = new RouterI();
}
