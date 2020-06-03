//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using ZeroC.Ice;

namespace ZeroC.Glacier2.Test.SessionHelper
{
    public sealed class Callback : ICallback
    {
        public void
        initiateCallback(ICallbackReceiverPrx? proxy, Current current) => proxy!.callback(current.Context);

        public void
        initiateCallbackEx(ICallbackReceiverPrx? proxy, Current current) => proxy!.callbackEx(current.Context);

        public void
        shutdown(Current current) => current.Adapter.Communicator.Shutdown();
    }
}
