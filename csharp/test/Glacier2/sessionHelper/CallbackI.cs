// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using ZeroC.Ice;

namespace ZeroC.Glacier2.Test.SessionHelper
{
    public sealed class Callback : ICallback
    {
        public void InitiateCallback(ICallbackReceiverPrx? proxy, Current current, CancellationToken cancel) =>
            proxy!.Callback(current.Context, cancel: cancel);

        public void InitiateCallbackEx(ICallbackReceiverPrx? proxy, Current current, CancellationToken cancel) =>
            proxy!.CallbackEx(current.Context, cancel: cancel);

        public void Shutdown(Current current, CancellationToken cancel) =>
            current.Adapter.Communicator.ShutdownAsync();
    }
}
