//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using ZeroC.Ice;

namespace ZeroC.Glacier2.Test.SessionHelper
{
    public sealed class Callback : ICallback
    {
        public void InitiateCallback(ICallbackReceiverPrx? proxy, Current current) =>
            proxy!.Callback(current.Context);

        public void InitiateCallbackEx(ICallbackReceiverPrx? proxy, Current current) =>
            proxy!.CallbackEx(current.Context);

        public void Shutdown(Current current) => current.Adapter.Communicator.ShutdownAsync();
    }
}
