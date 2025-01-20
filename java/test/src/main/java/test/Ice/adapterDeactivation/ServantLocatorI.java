// Copyright (c) ZeroC, Inc.

package test.Ice.adapterDeactivation;

import com.zeroc.Ice.ServantLocator;

public final class ServantLocatorI implements ServantLocator {
    static final class RouterI implements com.zeroc.Ice.Router {
        public com.zeroc.Ice.Router.GetClientProxyResult getClientProxy(
                com.zeroc.Ice.Current current) {
            return new com.zeroc.Ice.Router.GetClientProxyResult();
        }

        public com.zeroc.Ice.ObjectPrx getServerProxy(com.zeroc.Ice.Current current) {
            return com.zeroc.Ice.ObjectPrx.createProxy(
                    current.adapter.getCommunicator(), "dummy:tcp -h localhost -p 23456 -t 30000");
        }

        public com.zeroc.Ice.ObjectPrx[] addProxies(
                com.zeroc.Ice.ObjectPrx[] proxies, com.zeroc.Ice.Current current) {
            return null;
        }
    }

    public ServantLocatorI() {
        _deactivated = false;
    }

    @SuppressWarnings("deprecation")
    @Override
    protected synchronized void finalize() throws Throwable {
        test(_deactivated);
    }

    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public ServantLocator.LocateResult locate(com.zeroc.Ice.Current current) {
        synchronized (this) {
            test(!_deactivated);
        }

        if (current.id.name.equals("router")) {
            return new ServantLocator.LocateResult(_router, null);
        }

        test(current.id.category.isEmpty());
        test(current.id.name.equals("test"));

        return new ServantLocator.LocateResult(new TestI(), new Cookie());
    }

    public void finished(
            com.zeroc.Ice.Current current, com.zeroc.Ice.Object servant, java.lang.Object cookie) {
        synchronized (this) {
            test(!_deactivated);
        }

        if (current.id.name.equals("router")) {
            return;
        }

        Cookie co = (Cookie) cookie;
        test(co.message().equals("blahblah"));
    }

    public synchronized void deactivate(String category) {
        synchronized (this) {
            test(!_deactivated);

            _deactivated = true;
        }
    }

    private boolean _deactivated;
    private com.zeroc.Ice.Object _router = new RouterI();
}
