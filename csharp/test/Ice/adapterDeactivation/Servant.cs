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
        public (IObjectPrx?, bool?) GetClientProxy(Current current) => (null, false);

        public IObjectPrx GetServerProxy(Current current) =>
            IObjectPrx.Parse($"dummy:tcp -h localhost -p {_nextPort++} -t 30000", current.Adapter.Communicator);

        public IObjectPrx?[] AddProxies(IObjectPrx?[] proxies, Current current) => Array.Empty<IObjectPrx?>();

        private int _nextPort = 23456;
    }

    public sealed class Servant : IObject
    {
        private readonly IRouter _router = new Router();

        public ValueTask<OutgoingResponseFrame> DispatchAsync(IncomingRequestFrame request, Current current)
        {
            IObject? servant;
            if (current.Identity.Name.Equals("router"))
            {
                servant = _router;
            }
            else
            {
                test(current.Identity.Category.Length == 0);
                test(current.Identity.Name.Equals("test"));
                servant = new TestIntf();
            }
            return servant.DispatchAsync(request, current);
        }

        private static void test(bool b)
        {
            if (!b)
            {
                System.Diagnostics.Debug.Assert(false);
                throw new System.Exception();
            }
        }
    }
}
