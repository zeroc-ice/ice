// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using ZeroC.Ice;
using ZeroC.Test;

namespace ZeroC.Glacier2.Test.Router
{
    public sealed class Callback : ICallback
    {
        public void InitiateCallback(ICallbackReceiverPrx? proxy, Current current, CancellationToken cancel)
        {
            try
            {
                proxy!.Callback(current.Context, cancel: cancel);
            }
            catch (RemoteException ex)
            {
                ex.ConvertToUnhandled = false;
                throw;
            }
        }
        public void InitiateCallbackEx(ICallbackReceiverPrx? proxy, Current current, CancellationToken cancel)
        {
            try
            {
                proxy!.CallbackEx(current.Context, cancel: cancel);
            }
            catch (RemoteException ex)
            {
                ex.ConvertToUnhandled = false;
                throw;
            }
        }

        public void Shutdown(Current current, CancellationToken cancel) =>
            current.Communicator.ShutdownAsync();
    }

    public sealed class CallbackReceiver : ICallbackReceiver
    {
        private readonly object _mutex = new();
        private bool _callback;

        public CallbackReceiver() => _callback = false;

        public void Callback(Current current, CancellationToken cancel)
        {
            lock (_mutex)
            {
                TestHelper.Assert(!_callback);
                _callback = true;
                Monitor.Pulse(_mutex);
            }
        }

        public void CallbackEx(Current current, CancellationToken cancel)
        {
            Callback(current, cancel);
            throw new CallbackException(3.14, "3.14");
        }

        public void CallbackOK()
        {
            lock (_mutex)
            {
                while (!_callback)
                {
                    Monitor.Wait(_mutex, 30000);
                    TestHelper.Assert(_callback);
                }

                _callback = false;
            }
        }
    }
}
