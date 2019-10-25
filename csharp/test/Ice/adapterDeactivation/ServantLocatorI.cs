//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Text;

namespace Ice
{
    namespace adapterDeactivation
    {
        public class Cookie
        {
            public string message()
            {
                return "blahblah";
            }
        }

        public class RouterI : Ice.RouterDisp_
        {
            public override Ice.ObjectPrx getClientProxy(out Ice.Optional<bool> hasRoutingTable,
                Ice.Current current)
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
                lock (this)
                {
                    test(_deactivated);
                }
            }

            private static void test(bool b)
            {
                if (!b)
                {
                    throw new System.Exception();
                }
            }

            public Ice.Object locate(Ice.Current current, out System.Object cookie)
            {
                lock (this)
                {
                    test(!_deactivated);
                }

                if (current.id.name.Equals("router"))
                {
                    cookie = null;
                    return _router;
                }

                test(current.id.category.Length == 0);
                test(current.id.name.Equals("test"));

                cookie = new Cookie();

                return new TestI();
            }

            public void finished(Current current, Object servant, object cookie)
            {
                lock (this)
                {
                    test(!_deactivated);
                }

                if (current.id.name.Equals("router"))
                {
                    return;
                }

                var co = (Cookie)cookie;
                test(co.message().Equals("blahblah"));
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
