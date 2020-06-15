//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.AMI
{
    public class TestIntf : ITestIntf
    {
        public void
        op(Current current)
        {
        }

        public int opWithResult(Current current) => 15;

        public void opWithUE(Current current) => throw new TestIntfException();

        public void
        opWithPayload(byte[] seq, Current current)
        {
        }

        public void
        close(CloseMode mode, Current current) => current.Connection!.Close((ConnectionClose)(int)mode);

        public void sleep(int ms, Current current) => Thread.Sleep(ms);

        public void
        shutdown(Current current)
        {
            lock (this)
            {
                _shutdown = true;
                if (_pending != null)
                {
                    _pending.SetResult(null);
                    _pending = null;
                }
                current.Adapter.Communicator.Shutdown();
            }
        }

        public bool supportsAMD(Current current) => true;

        public bool supportsFunctionalTests(Current current) => false;

        public async ValueTask opAsyncDispatchAsync(Current current) => await Task.Delay(10);

        public async ValueTask<int> opWithResultAsyncDispatchAsync(Current current)
        {
            await Task.Delay(10);
            return await Self(current).opWithResultAsync();
        }

        public async ValueTask opWithUEAsyncDispatchAsync(Current current)
        {
            await Task.Delay(10);
            try
            {
                await Self(current).opWithUEAsync();
            }
            catch (RemoteException ex)
            {
                ex.ConvertToUnhandled = false;
                throw;
            }
        }

        private ITestIntfPrx Self(Current current) =>
            current.Adapter.CreateProxy(current.Identity, ITestIntfPrx.Factory);

        public ValueTask startDispatchAsync(Current current)
        {
            lock (this)
            {
                if (_shutdown)
                {
                    // Ignore, this can occur with the forceful connection close test, shutdown can be dispatch
                    // before start dispatch.
                    var v = new TaskCompletionSource<object?>();
                    v.SetResult(null);
                    return new ValueTask(v.Task);
                }
                else if (_pending != null)
                {
                    _pending.SetResult(null);
                }
                _pending = new TaskCompletionSource<object?>();
                return new ValueTask(_pending.Task);
            }
        }

        public void finishDispatch(Current current)
        {
            lock (this)
            {
                if (_shutdown)
                {
                    return;
                }
                else if (_pending != null) // Pending might not be set yet if startDispatch is dispatch out-of-order
                {
                    _pending.SetResult(null);
                    _pending = null;
                }
            }
        }

        public int set(int value, Current current)
        {
            return Interlocked.Exchange(ref _value, value);
        }

        private bool _shutdown;
        private TaskCompletionSource<object?>? _pending = null;
        private int _value;
    }

    public class TestIntf2 : Outer.Inner.ITestIntf
    {
        public (int, int) op(int i, Current current) => (i, i);
    }
}
