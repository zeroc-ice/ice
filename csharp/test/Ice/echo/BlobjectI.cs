//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using System.Diagnostics;
using Ice;

public class BlobjectI : IObject
{
    public async ValueTask<OutputStream> DispatchAsync(InputStream inputStream, Current current)
    {
        Debug.Assert(current.Connection != null);

        var incomingRequestFrame = new IncomingRequestFrame(inputStream, current);

        var prx = current.Connection.CreateProxy(current.Id, IObjectPrx.Factory).Clone(facet: current.Facet,
            oneway: current.IsOneway);

        var requestFrame = OutgoingRequestFrame.Create(prx, current.Operation,
            current.IsIdempotent, incomingRequestFrame.TakePayload(), current.Context);

        IncomingResponseFrame responseFrame = await prx.InvokeAsync(requestFrame);
        return OutgoingResponseFrame.Create(responseFrame.ReplyStatus, responseFrame.TakePayload(), current);
    }
}
