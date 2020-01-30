//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Text;

namespace Ice.adapterDeactivation
{
    public class Cookie
    {
        public string message() => "blahblah";
    }

    public class Router : IRouter
    {
        public IRouter.GetClientProxyReturnValue GetClientProxy(Current current) =>
            new IRouter.GetClientProxyReturnValue(null, false);

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

    public sealed class ServantLocator : IServantLocator
    {
        public ServantLocator() => _deactivated = false;

        ~ServantLocator()
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

        public Disp Locate(Current current, out object cookie)
        {
            lock (this)
            {
                test(!_deactivated);
            }

            if (current.Id.Name.Equals("router"))
            {
                cookie = null;
                return (_router as IObject).Dispatch;
            }

            test(current.Id.Category.Length == 0);
            test(current.Id.Name.Equals("test"));

            cookie = new Cookie();

            var testI = new TestIntf();
            return (testI as IObject).Dispatch;
        }

        public void Finished(Current current, Disp servant, object cookie)
        {
            lock (this)
            {
                test(!_deactivated);
            }

            if (current.Id.Name.Equals("router"))
            {
                return;
            }

            var co = (Cookie)cookie;
            test(co.message().Equals("blahblah"));
        }

        public void Deactivate(string category)
        {
            lock (this)
            {
                test(!_deactivated);
                _deactivated = true;
            }
        }

        private bool _deactivated;
        private Router _router = new Router();
    }
}
