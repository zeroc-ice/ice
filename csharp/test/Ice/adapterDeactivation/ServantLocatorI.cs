// Copyright (c) ZeroC, Inc.

using System.Text;

namespace Ice
{
    namespace adapterDeactivation
    {
        public class RouterI : Ice.RouterDisp_
        {
            public override Ice.ObjectPrx getClientProxy(out bool? hasRoutingTable, Current current)
            {
                hasRoutingTable = false;
                return null;
            }

            public override Ice.ObjectPrx getServerProxy(Current current) =>
                ObjectPrxHelper.createProxy(current.adapter.getCommunicator(),
                "dummy:tcp -h localhost -p 23456 -t 30000");

            public override Ice.ObjectPrx[] addProxies(ObjectPrx[] proxies, Current current) => null;
        }

        public sealed class ServantLocatorI : Ice.ServantLocator
        {
            public ServantLocatorI()
            {
                _deactivated = false;
            }

            ~ServantLocatorI()
            {
                lock (this)
                {
                    test(_deactivated);
                }
            }

            private static void test(bool b) => global::Test.TestHelper.test(b);

            public Ice.Object locate(Ice.Current current, out System.Object cookie)
            {
                lock (this)
                {
                    test(!_deactivated);
                }

                if (current.id.name == "router")
                {
                    cookie = null;
                    return _router;
                }

                test(current.id.category.Length == 0);
                test(current.id.name == "test");

                cookie = new Cookie();

                return new TestI();
            }

            public void finished(Ice.Current current, Ice.Object servant, System.Object cookie)
            {
                lock (this)
                {
                    test(!_deactivated);
                }

                if (current.id.name == "router")
                {
                    return;
                }

                Cookie co = (Cookie)cookie;
                test(co.message() == "blahblah");
            }

            public void deactivate(string category)
            {
                lock (this)
                {
                    test(!_deactivated);

                    _deactivated = true;
                }
            }

            private bool _deactivated;
            private RouterI _router = new RouterI();
        }
    }
}
