//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using System.Diagnostics;
using Ice;

public class BlobjectI : IObject
{
    public ValueTask<OutputStream> DispatchAsync(InputStream inputStream, Current current)
    {
        var request = new IncomingRequestFrame(inputStream, current); // Temporary

        Debug.Assert(current.Connection != null);
        var proxy = current.Connection.CreateProxy(current.Id, IObjectPrx.Factory).Clone(facet: current.Facet,
            oneway: current.IsOneway);

        return proxy.ForwardAsync(request);
    }
}
