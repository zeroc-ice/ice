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

        public class RouterI : Router
        {
            public Router.GetClientProxyReturnValue GetClientProxy(Current current) =>
                new Router.GetClientProxyReturnValue(null, false);

            public IObjectPrx GetServerProxy(Current current)
            {
                StringBuilder s = new StringBuilder("dummy:tcp -h localhost -p ");
                s.Append(_nextPort++);
                s.Append(" -t 30000");
                return IObjectPrx.Parse(s.ToString(), current.Adapter.Communicator);
            }

            public IObjectPrx[] AddProxies(IObjectPrx[] proxies, Current current)
            {
                return null;
            }

            private int _nextPort = 23456;
        }

        public sealed class ServantLocatorI : ServantLocator
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

            public Disp locate(Current current, out object cookie)
            {
                lock (this)
                {
                    test(!_deactivated);
                }

                if (current.Id.name.Equals("router"))
                {
                    cookie = null;
                    RouterTraits routerT = default;
                    return (incoming, current) => routerT.Dispatch(_router, incoming, current);
                }

                test(current.Id.category.Length == 0);
                test(current.Id.name.Equals("test"));

                cookie = new Cookie();

                var testT = default(Test.TestIntfTraits);
                var testI = new TestI();
                return (incoming, current) => testT.Dispatch(testI, incoming, current);
            }

            public void finished(Current current, Disp servant, object cookie)
            {
                lock (this)
                {
                    test(!_deactivated);
                }

                if (current.Id.name.Equals("router"))
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
