//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace Ice
{
    namespace ami
    {
        public class TestI : Test.TestIntf
        {
            protected static void test(bool b)
            {
                if (!b)
                {
                    Debug.Assert(false);
                    throw new System.Exception();
                }
            }

            public TestI()
            {
            }

            public void
            op(Ice.Current current)
            {
            }

            public int
            opWithResult(Ice.Current current)
            {
                return 15;
            }

            public void
            opWithUE(Ice.Current current)
            {
                throw new Test.TestIntfException();
            }

            public void
            opWithPayload(byte[] seq, Ice.Current current)
            {
            }

            public void
            close(Test.CloseMode mode, Ice.Current current)
            {
                current.Connection.close((Ice.ConnectionClose)((int)mode));
            }

            public void
            sleep(int ms, Ice.Current current)
            {
                Thread.Sleep(ms);
            }

            public void
            shutdown(Ice.Current current)
            {
                lock (this)
                {
                    _shutdown = true;
                    if (_pending != null)
                    {
                        _pending.SetResult(null);
                        _pending = null;
                    }
                    current.Adapter.Communicator.shutdown();
                }
            }

            public bool
            supportsAMD(Ice.Current current)
            {
                return true;
            }

            public bool
            supportsFunctionalTests(Ice.Current current)
            {
                return false;
            }

            public async Task
            opAsyncDispatchAsync(Ice.Current current)
            {
                await Task.Delay(10);
            }

            public async Task<int>
            opWithResultAsyncDispatchAsync(Ice.Current current)
            {
                await Task.Delay(10);
                test(Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Server"));
                var r = await self(current).opWithResultAsync();
                test(Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Server"));
                return r;
            }

            public async Task
            opWithUEAsyncDispatchAsync(Ice.Current current)
            {
                test(Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Server"));
                await Task.Delay(10);
                test(Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Server"));
                await self(current).opWithUEAsync();
            }

            public void
            pingBiDir(Test.PingReplyPrx reply, Ice.Current current)
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

            Test.TestIntfPrx
            self(Ice.Current current)
            {
                return Test.TestIntfPrx.UncheckedCast(current.Adapter.CreateProxy(current.Id));
            }

            public Task
            startDispatchAsync(Ice.Current current)
            {
                lock (this)
                {
                    if (_shutdown)
                    {
                        // Ignore, this can occur with the forcefull connection close test, shutdown can be dispatch
                        // before start dispatch.
                        var v = new TaskCompletionSource<object>();
                        v.SetResult(null);
                        return v.Task;
                    }
                    else if (_pending != null)
                    {
                        _pending.SetResult(null);
                    }
                    _pending = new TaskCompletionSource<object>();
                    return _pending.Task;
                }
            }

            public void
            finishDispatch(Ice.Current current)
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

        public class TestII : Test.Outer.Inner.TestIntf
        {
            public int
            op(int i, out int j, Ice.Current current)
            {
                j = i;
                return i;
            }
        }

        public class TestControllerI : Test.TestIntfController
        {
            public void
            holdAdapter(Ice.Current current)
            {
                _adapter.Hold();
            }

            public void
            resumeAdapter(Ice.Current current)
            {
                _adapter.Activate();
            }

            public
            TestControllerI(Ice.ObjectAdapter adapter)
            {
                _adapter = adapter;
            }

            private Ice.ObjectAdapter _adapter;
        }
    }
}
