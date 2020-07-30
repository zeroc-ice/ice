//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using ZeroC.Ice;
using Test;

namespace ZeroC.Glacier2.Test.Router
{
    public sealed class Callback : ICallback
    {
        public void InitiateCallback(ICallbackReceiverPrx? proxy, Current current) =>
            proxy!.Callback(current.Context);

        public void InitiateCallbackEx(ICallbackReceiverPrx? proxy, Current current)
        {
            try
            {
                proxy!.CallbackEx(current.Context);
            }
            catch (RemoteException ex)
            {
                ex.ConvertToUnhandled = false;
                throw;
            }
        }

        public void Shutdown(Current current) => current.Adapter.Communicator.ShutdownAsync();
    }

    public sealed class CallbackReceiver : ICallbackReceiver
    {
        private readonly object _mutex = new object();
        private bool _callback;

        public CallbackReceiver() => _callback = false;

        public void Callback(Current current)
        {
            lock (_mutex)
            {
                TestHelper.Assert(!_callback);
                _callback = true;
                System.Threading.Monitor.Pulse(_mutex);
            }
        }

        public void CallbackEx(Current current)
        {
            Callback(current);
            throw new CallbackException(3.14, "3.14");
        }

        public void CallbackOK()
        {
            lock (_mutex)
            {
                while (!_callback)
                {
                    System.Threading.Monitor.Wait(_mutex, 30000);
                    TestHelper.Assert(_callback);
                }

                _callback = false;
            }
        }
    }
}
