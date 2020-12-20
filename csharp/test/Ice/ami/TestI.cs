// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.AMI
{
    public class TestIntf : ITestIntf
    {
        private readonly object _mutex = new();
        private bool _shutdown;
        private TaskCompletionSource<object?>? _pending;
        private int _value;

        public void Op(Current current, CancellationToken cancel)
        {
        }

        public int OpWithResult(Current current, CancellationToken cancel) => 15;

        public void OpWithUE(Current current, CancellationToken cancel) => throw new TestIntfException();

        public void OpWithPayload(byte[] seq, Current current, CancellationToken cancel)
        {
        }

        public void Close(CloseMode mode, Current current, CancellationToken cancel)
        {
            if (mode == CloseMode.Gracefully)
            {
                current.Connection.GoAwayAsync(cancel: cancel);
            }
            else
            {
                current.Connection.AbortAsync();
            }
        }

        public void Sleep(int ms, Current current, CancellationToken cancel)
        {
            try
            {
                Task.Delay(ms, cancel).Wait(cancel);
                // Cancellation isn't supported with Ice1
                TestHelper.Assert(!current.Context.ContainsKey("cancel") ||
                                  current.Context["cancel"] == "mightSucceed" ||
                                  current.Protocol == Protocol.Ice1);
            }
            catch (System.AggregateException ex) when (ex.InnerException is TaskCanceledException)
            {
                // Expected if the request is canceled.
                TestHelper.Assert(current.Context.ContainsKey("cancel"));
            }
        }

        public void Shutdown(Current current, CancellationToken cancel)
        {
            lock (_mutex)
            {
                _shutdown = true;
                if (_pending != null)
                {
                    _pending.SetResult(null);
                    _pending = null;
                }
                current.Communicator.ShutdownAsync();
            }
        }

        public bool SupportsAMD(Current current, CancellationToken cancel) => true;

        public bool SupportsFunctionalTests(Current current, CancellationToken cancel) => false;

        public async ValueTask OpAsyncDispatchAsync(Current current, CancellationToken cancel) =>
            await Task.Delay(10, cancel);

        public async ValueTask<int> OpWithResultAsyncDispatchAsync(Current current, CancellationToken cancel)
        {
            await Task.Delay(10, cancel);
            return await Self(current).OpWithResultAsync(cancel: cancel);
        }

        public async ValueTask OpWithUEAsyncDispatchAsync(Current current, CancellationToken cancel)
        {
            await Task.Delay(10, cancel);
            try
            {
                await Self(current).OpWithUEAsync(cancel: cancel);
            }
            catch (RemoteException ex)
            {
                ex.ConvertToUnhandled = false;
                throw;
            }
        }

        private static ITestIntfPrx Self(Current current) =>
            current.Adapter.CreateProxy(current.Identity, ITestIntfPrx.Factory);

        public ValueTask StartDispatchAsync(Current current, CancellationToken cancel)
        {
            lock (_mutex)
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

        public void FinishDispatch(Current current, CancellationToken cancel)
        {
            lock (_mutex)
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

        public int Set(int newValue, Current current, CancellationToken cancel)
        {
            int oldValue = _value;
            _value = newValue;
            return oldValue;
        }

        public void SetOneway(int previousValue, int newValue, Current current, CancellationToken cancel)
        {
            if (_value != previousValue)
            {
                System.Console.Error.WriteLine($"previous value `{_value}' is not the expected value `{previousValue}'");
            }
            TestHelper.Assert(_value == previousValue);
            _value = newValue;
        }
    }

    public class TestIntf2 : Outer.Inner.ITestIntf
    {
        public (int, int) Op(int i, Current current, CancellationToken cancel) => (i, i);
    }
}
