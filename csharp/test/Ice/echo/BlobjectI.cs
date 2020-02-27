//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using System.Diagnostics;
using Ice;

public class BlobjectI : IObject
{
    public async ValueTask<OutputStream> DispatchAsync(InputStream incomingRequestFrame, Current current)
    {
        Debug.Assert(current.Connection != null);

        var prx = current.Connection.CreateProxy(current.Id, IObjectPrx.Factory).Clone(facet: current.Facet,
            oneway: current.IsOneway);

        var requestFrame = OutgoingRequestFrame.FromIncoming(incomingRequestFrame, prx, current.Operation,
            current.IsIdempotent, current.Context);
        IncomingResponseFrame responseFrame = await prx.InvokeAsync(requestFrame);
        return OutgoingResponseFrame.FromIncoming(responseFrame, current);
    }
}
