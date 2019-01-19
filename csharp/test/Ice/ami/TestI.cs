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
        public class TestI : Test.TestIntfDisp_
        {
            protected static void test(bool b)
            {
                if(!b)
                {
                    Debug.Assert(false);
                    throw new System.Exception();
                }
            }

            public TestI()
            {
            }

            override public void
            op(Ice.Current current)
            {
            }

            override public int
            opWithResult(Ice.Current current)
            {
                return 15;
            }

            override public void
            opWithUE(Ice.Current current)
            {
                throw new Test.TestIntfException();
            }

            override public void
            opWithPayload(byte[] seq, Ice.Current current)
            {
            }

            override public void
            opBatch(Ice.Current current)
            {
                lock(this)
                {
                    ++_batchCount;
                    Monitor.Pulse(this);
                }
            }

            override public int
            opBatchCount(Ice.Current current)
            {
                lock(this)
                {
                    return _batchCount;
                }
            }

            override public bool
            waitForBatch(int count, Ice.Current current)
            {
                lock(this)
                {
                    while(_batchCount < count)
                    {
                        Monitor.Wait(this, 10000);
                    }
                    bool result = count == _batchCount;
                    _batchCount = 0;
                    return result;
                }
            }

            override public void
            close(Test.CloseMode mode, Ice.Current current)
            {
                current.con.close((Ice.ConnectionClose)((int)mode));
            }

            override public void
            sleep(int ms, Ice.Current current)
            {
                Thread.Sleep(ms);
            }

            override public void
            shutdown(Ice.Current current)
            {
                lock(this)
                {
                    _shutdown = true;
                    if(_pending != null)
                    {
                        _pending.SetResult(null);
                        _pending = null;
                    }
                    current.adapter.getCommunicator().shutdown();
                }
            }

            override public bool
            supportsAMD(Ice.Current current)
            {
                return true;
            }

            override public bool
            supportsFunctionalTests(Ice.Current current)
            {
                return false;
            }

            override public async Task
            opAsyncDispatchAsync(Ice.Current current)
            {
                await Task.Delay(10);
            }

            override public async Task<int>
            opWithResultAsyncDispatchAsync(Ice.Current current)
            {
                await Task.Delay(10);
                test(Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Server"));
                var r = await self(current).opWithResultAsync();
                test(Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Server"));
                return r;
            }

            override public async Task
            opWithUEAsyncDispatchAsync(Ice.Current current)
            {
                test(Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Server"));
                await Task.Delay(10);
                test(Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Server"));
                await self(current).opWithUEAsync();
            }

            override public void
            pingBiDir(Test.PingReplyPrx reply, Ice.Current current)
            {
                reply = Test.PingReplyPrxHelper.uncheckedCast(reply.ice_fixed(current.con));
                Thread dispatchThread = Thread.CurrentThread;
                reply.replyAsync().ContinueWith(
                   (t) =>
                    {
                        Thread callbackThread = Thread.CurrentThread;
                        test(dispatchThread != callbackThread);
                        test(callbackThread.Name.Contains("Ice.ThreadPool.Server"));
                    },
                    reply.ice_scheduler()).Wait();
            }

            Test.TestIntfPrx
            self(Ice.Current current)
            {
                return Test.TestIntfPrxHelper.uncheckedCast(current.adapter.createProxy(current.id));
            }

            override public Task
            startDispatchAsync(Ice.Current current)
            {
                lock(this)
                {
                    if(_shutdown)
                    {
                        // Ignore, this can occur with the forcefull connection close test, shutdown can be dispatch
                        // before start dispatch.
                        var v = new TaskCompletionSource<object>();
                        v.SetResult(null);
                        return v.Task;
                    }
                    else if(_pending != null)
                    {
                        _pending.SetResult(null);
                    }
                    _pending = new TaskCompletionSource<object>();
                    return _pending.Task;
                }
            }

            override public void
            finishDispatch(Ice.Current current)
            {
                lock(this)
                {
                    if(_shutdown)
                    {
                        return;
                    }
                    else if(_pending != null) // Pending might not be set yet if startDispatch is dispatch out-of-order
                    {
                        _pending.SetResult(null);
                        _pending = null;
                    }
                }
            }

            private int _batchCount;
            private bool _shutdown;
            private TaskCompletionSource<object> _pending = null;
        }

        public class TestII : Test.Outer.Inner.TestIntfDisp_
        {
            override public int
            op(int i, out int j, Ice.Current current)
            {
                j = i;
                return i;
            }
        }

        public class TestControllerI : Test.TestIntfControllerDisp_
        {
            override public void
            holdAdapter(Ice.Current current)
            {
                _adapter.hold();
            }

            override public void
            resumeAdapter(Ice.Current current)
            {
                _adapter.activate();
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
