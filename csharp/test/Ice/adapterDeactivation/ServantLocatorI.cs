// Copyright (c) ZeroC, Inc.

namespace Ice.adapterDeactivation;

public class RouterI : Ice.RouterDisp_
{
    public override Ice.ObjectPrx getClientProxy(out bool? hasRoutingTable, Current current)
    {
        hasRoutingTable = false;
        return null;
    }

    public override Ice.ObjectPrx getServerProxy(Current current) =>
        ObjectPrxHelper.createProxy(current.adapter.getCommunicator(), "dummy:tcp -h localhost -p 23456 -t 30000");

    public override Ice.ObjectPrx[] addProxies(ObjectPrx[] proxies, Current current) => null;
}

public sealed class ServantLocatorI : ServantLocator
{
    public ServantLocatorI() => _deactivated = false;

    private static void test(bool b) => global::Test.TestHelper.test(b);

    public Object locate(Current curr, out object cookie)
    {
        lock (_mutex)
        {
            test(!_deactivated);
        }

        if (curr.id.name == "router")
        {
            cookie = null;
            return _router;
        }

        test(curr.id.category.Length == 0);
        test(curr.id.name == "test");

        cookie = new Cookie();

        return new TestI();
    }

    public void finished(Current curr, Object servant, object cookie)
    {
        lock (_mutex)
        {
            test(!_deactivated);
        }

        if (curr.id.name == "router")
        {
            return;
        }

        var co = (Cookie)cookie;
        test(co.message() == "blahblah");
    }

    public void deactivate(string category)
    {
        lock (_mutex)
        {
            test(!_deactivated);

            _deactivated = true;
        }
    }

    private bool _deactivated;
    private readonly RouterI _router = new();
    private readonly object _mutex = new();
}
