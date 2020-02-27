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

        var requestFrame = new OutgoingRequestFrame(prx, current.Operation, current.IsIdempotent, current.Context,
            incomingRequestFrame.TakePayload());

        IncomingResponseFrame responseFrame = await prx.InvokeAsync(requestFrame);
        return new OutgoingResponseFrame(current, responseFrame.ReplyStatus, responseFrame.TakePayload());
    }
}
