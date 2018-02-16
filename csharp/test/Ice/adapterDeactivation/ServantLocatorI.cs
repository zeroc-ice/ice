// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;
using System.Text;

public class RouterI : Ice.RouterDisp_
{
    public override Ice.ObjectPrx getClientProxy(out Ice.Optional<bool> hasRoutingTable, Ice.Current current)
    {
        hasRoutingTable = false;
        return null;
    }

    public override Ice.ObjectPrx getServerProxy(Ice.Current current)
    {
        StringBuilder s = new StringBuilder("dummy:tcp -h localhost -p ");
        s.Append(_nextPort++);
        s.Append(" -t 30000");
        return current.adapter.getCommunicator().stringToProxy(s.ToString());
    }

    public override Ice.ObjectPrx[] addProxies(Ice.ObjectPrx[] proxies, Ice.Current current)
    {
        return null;
    }

    private int _nextPort = 23456;
}

public sealed class ServantLocatorI : Ice.ServantLocator
{
    public ServantLocatorI()
    {
        _deactivated = false;
    }

    ~ServantLocatorI()
    {
        lock(this)
        {
            test(_deactivated);
        }
    }

    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
        }
    }

    public Ice.Object locate(Ice.Current current, out System.Object cookie)
    {
        lock(this)
        {
            test(!_deactivated);
        }

        if(current.id.name.Equals("router"))
        {
            cookie = null;
            return _router;
        }

        test(current.id.category.Length == 0);
        test(current.id.name.Equals("test"));

        cookie = new CookieI();

        return new TestI();
    }

    public void finished(Ice.Current current, Ice.Object servant, System.Object cookie)
    {
        lock(this)
        {
            test(!_deactivated);
        }

        if(current.id.name.Equals("router"))
        {
            return;
        }

        Cookie co = (Cookie) cookie;
        test(co.message().Equals("blahblah"));
    }

    public void deactivate(string category)
    {
        lock(this)
        {
            test(!_deactivated);

            _deactivated = true;
        }
    }

    private bool _deactivated;
    static private RouterI _router = new RouterI();
}
