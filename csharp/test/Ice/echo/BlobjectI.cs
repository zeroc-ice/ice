//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;
using Ice;

public class BlobjectI : IObject
{
    public ValueTask<OutgoingResponseFrame> DispatchAsync(IncomingRequestFrame request, Current current)
    {
        TestHelper.Assert(current.Connection != null);
        IObjectPrx proxy = current.Connection.CreateProxy(current.Identity, IObjectPrx.Factory).Clone(current.Facet,
            IObjectPrx.Factory, oneway: current.IsOneway);
        return proxy.ForwardAsync(current.IsOneway, request);
    }
}
