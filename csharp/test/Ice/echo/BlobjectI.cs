//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using System.Diagnostics;
using Ice;

public class BlobjectI : IObject
{
    public ValueTask<OutgoingResponseFrame> DispatchAsync(IncomingRequestFrame request, Current current)
    {
        Debug.Assert(current.Connection != null);
        IObjectPrx proxy = current.Connection.CreateProxy(current.Id, IObjectPrx.Factory).Clone(facet: current.Facet,
            oneway: current.IsOneway);
        return proxy.ForwardAsync(current.IsOneway, request);
    }
}
