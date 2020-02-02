//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System;
using System.Text;
using System.Threading.Tasks;

namespace Ice.adapterDeactivation
{
    public sealed class Router : IRouter
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

    public sealed class Servant : IObject
    {
        private IRouter _router = new Router();

        public Task<OutputStream?>? Dispatch(IceInternal.Incoming inS, Current current)
        {
            IObject? servant = null;
            if (current.Id.Name.Equals("router"))
            {
                servant = _router;
            }
            else
            {
                test(current.Id.Category.Length == 0);
                test(current.Id.Name.Equals("test"));
                servant = new TestIntf();
            }
            return servant.Dispatch(inS, current);
        }

        private static void test(bool b)
        {
            if (!b)
            {
                throw new System.Exception();
            }
        }
    }
}
