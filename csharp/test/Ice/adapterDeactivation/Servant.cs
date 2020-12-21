// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.AdapterDeactivation
{
    public sealed class Router : IRouter
    {
        public (IObjectPrx?, bool?) GetClientProxy(Current current, CancellationToken cancel) => (null, false);

        public IObjectPrx GetServerProxy(Current current, CancellationToken cancel) =>
            IObjectPrx.Parse(TestHelper.GetTestProtocol(current.Communicator.GetProperties()) == Protocol.Ice1 ?
                $"dummy:tcp -h localhost -p 23456" : $"ice+tcp://localhost:23456/dummy",
                current.Communicator);

        public IEnumerable<IObjectPrx?> AddProxies(IObjectPrx?[] proxies, Current current, CancellationToken cancel) =>
            Array.Empty<IObjectPrx?>();
    }

    public sealed class Servant : IObject
    {
        private readonly IRouter _router = new Router();

        public ValueTask<OutgoingResponseFrame> DispatchAsync(
            IncomingRequestFrame request,
            Current current,
            CancellationToken cancel)
        {
            IObject? servant;
            if (current.Identity.Name == "router")
            {
                servant = _router;
            }
            else
            {
                TestHelper.Assert(current.Identity.Category.Length == 0);
                TestHelper.Assert(current.Identity.Name == "test");
                servant = new TestIntf();
            }
            return servant.DispatchAsync(request, current, cancel);
        }
    }
}
