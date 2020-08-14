//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    public delegate ValueTask<OutgoingResponseFrame> Dispatcher(IncomingRequestFrame request, Current current);

    public delegate ValueTask<OutgoingResponseFrame> DispatchInterceptor(
        IncomingRequestFrame request,
        Current current,
        Dispatcher next);
}
