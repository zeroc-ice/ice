//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    public delegate ValueTask<IncomingResponseFrame> Invoker(IObjectPrx target, OutgoingRequestFrame request);

    public delegate ValueTask<IncomingResponseFrame> InvocationInterceptor(
        IObjectPrx target,
        OutgoingRequestFrame request,
        Invoker next);
}
