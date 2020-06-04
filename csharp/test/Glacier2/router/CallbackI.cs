//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using ZeroC.Ice;
using Test;

namespace ZeroC.Glacier2.Test.Router
{
    public sealed class Callback : ICallback
    {
        public void
        initiateCallback(ICallbackReceiverPrx? proxy, Current current) => proxy!.callback(current.Context);

        public void
        initiateCallbackEx(ICallbackReceiverPrx? proxy, Current current)
        {
            try
            {
                proxy!.callbackEx(current.Context);
            }
            catch (RemoteException ex)
            {
                ex.ConvertToUnhandled = false;
                throw;
            }
        }

        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();
    }

    public sealed class CallbackReceiver : ICallbackReceiver
    {
        public CallbackReceiver() => _callback = false;

        public void
        callback(Current current)
        {
            lock (this)
            {
                TestHelper.Assert(!_callback);
                _callback = true;
                System.Threading.Monitor.Pulse(this);
            }
        }

        public void
        callbackEx(Current current)
        {
            callback(current);
            throw new CallbackException(3.14, "3.14");
        }

        public void
        callbackOK()
        {
            lock (this)
            {
                while (!_callback)
                {
                    System.Threading.Monitor.Wait(this, 30000);
                    TestHelper.Assert(_callback);
                }

                _callback = false;
            }
        }

        private bool _callback;
    }
}
