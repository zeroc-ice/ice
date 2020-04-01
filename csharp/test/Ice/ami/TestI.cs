//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace Ice.ami
{
    public class TestIntf : Test.ITestIntf
    {
        protected static void test(bool b)
        {
            if (!b)
            {
                Debug.Assert(false);
                throw new System.Exception();
            }
        }

        public void
        op(Current current)
        {
        }

        public int opWithResult(Current current) => 15;

        public void opWithUE(Current current) => throw new Test.TestIntfException();

        public void
        opWithPayload(byte[] seq, Ice.Current current)
        {
        }

        public void
        close(Test.CloseMode mode, Current current)
        {
            current.Connection.Close((ConnectionClose)((int)mode));
        }

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

        public async ValueTask<int> opWithResultAsyncDispatchAsync(Ice.Current current)
        {
            await Task.Delay(10);
            test(Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Server"));
            var r = await self(current).opWithResultAsync();
            test(Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Server"));
            return r;
        }

        public async ValueTask opWithUEAsyncDispatchAsync(Ice.Current current)
        {
            test(Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Server"));
            await Task.Delay(10);
            test(Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Server"));
            try
            {
                await self(current).opWithUEAsync();
            }
            catch (RemoteException ex)
            {
                ex.ConvertToUnhandled = false;
                throw;
            }
        }

        public void pingBiDir(Test.IPingReplyPrx reply, Ice.Current current)
        {
            reply = reply.Clone(fixedConnection: current.Connection);
            Thread dispatchThread = Thread.CurrentThread;
            reply.replyAsync().ContinueWith(
                (t) =>
                {
                    Thread callbackThread = Thread.CurrentThread;
                    test(dispatchThread != callbackThread);
                    test(callbackThread.Name.Contains("Ice.ThreadPool.Server"));
                },
                reply.Scheduler).Wait();
        }

        Test.ITestIntfPrx self(Current current) =>
            current.Adapter.CreateProxy(current.Id, Test.ITestIntfPrx.Factory);

        public ValueTask startDispatchAsync(Ice.Current current)
        {
            lock (this)
            {
                if (_shutdown)
                {
                    // Ignore, this can occur with the forcefull connection close test, shutdown can be dispatch
                    // before start dispatch.
                    var v = new TaskCompletionSource<object>();
                    v.SetResult(null);
                    return new ValueTask(v.Task);
                }
                else if (_pending != null)
                {
                    _pending.SetResult(null);
                }
                _pending = new TaskCompletionSource<object>();
                return new ValueTask(_pending.Task);
            }
        }

        public void finishDispatch(Ice.Current current)
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

        private bool _shutdown;
        private TaskCompletionSource<object> _pending = null;
    }

    public class TestIntf2 : Test.Outer.Inner.ITestIntf
    {
        public (int, int) op(int i, Current current) => (i, i);
    }
}
