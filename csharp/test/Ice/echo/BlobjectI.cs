// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Echo
{
    public class BlobjectI : IObject
    {
        public ValueTask<OutgoingResponseFrame> DispatchAsync(
            IncomingRequestFrame request,
            Current current,
            CancellationToken cancel)
        {
            TestHelper.Assert(current.Connection != null);
            IObjectPrx proxy = current.Connection.CreateProxy(current.Identity, IObjectPrx.Factory).Clone(
                IObjectPrx.Factory,
                facet: current.Facet,
                oneway: current.IsOneway);
            return proxy.ForwardAsync(current.IsOneway, request, cancel: cancel);
        }
    }
}
