// Copyright (c) ZeroC, Inc.

package test.Ice.adapterDeactivation;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.Router;
import com.zeroc.Ice.ServantLocator;

public final class ServantLocatorI implements ServantLocator {
    static final class RouterI implements Router {
        public Router.GetClientProxyResult getClientProxy(
                Current current) {
            return new Router.GetClientProxyResult();
        }

        public ObjectPrx getServerProxy(Current current) {
            return ObjectPrx.createProxy(
                current.adapter.getCommunicator(), "dummy:tcp -h localhost -p 23456 -t 30000");
        }

        public ObjectPrx[] addProxies(
                ObjectPrx[] proxies, Current current) {
            return null;
        }
    }

    public ServantLocatorI() {
        _deactivated = false;
    }

    @SuppressWarnings({"nofinalizer", "deprecation"})
    @Override
    protected synchronized void finalize() throws Throwable {
        test(_deactivated);
    }

    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public ServantLocator.LocateResult locate(Current current) {
        synchronized (this) {
            test(!_deactivated);
        }

        if ("router".equals(current.id.name)) {
            return new ServantLocator.LocateResult(_router, null);
        }

        test(current.id.category.isEmpty());
        test("test".equals(current.id.name));

        return new ServantLocator.LocateResult(new TestI(), new Cookie());
    }

    public void finished(
            Current current, Object servant, java.lang.Object cookie) {
        synchronized (this) {
            test(!_deactivated);
        }

        if ("router".equals(current.id.name)) {
            return;
        }

        Cookie co = (Cookie) cookie;
        test("blahblah".equals(co.message()));
    }

    public synchronized void deactivate(String category) {
        synchronized (this) {
            test(!_deactivated);

            _deactivated = true;
        }
    }

    private boolean _deactivated;
    private final Object _router = new RouterI();
}
