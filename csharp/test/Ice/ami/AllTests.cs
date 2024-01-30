//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace Ice
{
    namespace ami
    {
        public class AllTests : global::Test.AllTests
        {
            public class PingReplyI : Test.PingReplyDisp_
            {
                public override void reply(Ice.Current current)
                {
                    _received = true;
                }

                public bool checkReceived()
                {
                    return _received;
                }

                private bool _received = false;
            }

            public class Progress : IProgress<bool>
            {
                public Progress(Action<bool> report)
                {
                    _report = report;
                }

                public void Report(bool sentSynchronously)
                {
                    _report(sentSynchronously);
                }

                Action<bool> _report;
            }

            public class ProgressCallback : IProgress<bool>
            {
                public bool Sent
                {
                    get
                    {
                        lock(this)
                        {
                            return _sent;
                        }
                    }
                    set
                    {
                        lock(this)
                        {
                            _sent = value;
                        }
                    }
                }

                public bool SentSynchronously
                {
                    get
                    {
                        lock(this)
                        {
                            return _sentSynchronously;
                        }
                    }
                    set
                    {
                        lock(this)
                        {
                            _sentSynchronously = value;
                        }
                    }
                }

                public void Report(bool sentSynchronously)
                {
                    SentSynchronously = sentSynchronously;
                    Sent = true;
                }

                private bool _sent = false;
                private bool _sentSynchronously = false;
            }

            private class CallbackBase
            {
                internal CallbackBase()
                {
                    _called = false;
                }

                public virtual void check()
                {
                    lock(this)
                    {
                        while(!_called)
                        {
                            Monitor.Wait(this);
                        }
                        _called = false;
                    }
                }

                public virtual void called()
                {
                    lock(this)
                    {
                        Debug.Assert(!_called);
                        _called = true;
                        Monitor.Pulse(this);
                    }
                }

                private bool _called;
            }

            private class SentCallback : CallbackBase
            {
                public SentCallback()
                {
                    _thread = Thread.CurrentThread;
                }

                public void
                sent(bool ss)
                {
                    test(ss && _thread == Thread.CurrentThread || !ss && _thread != Thread.CurrentThread);

                    called();
                }

                Thread _thread;
            }

            public static async Task allTestsAsync(global::Test.TestHelper helper, bool collocated)
            {
                Ice.Communicator communicator = helper.communicator();
                string sref = "test:" + helper.getTestEndpoint(0);
                Ice.ObjectPrx obj = communicator.stringToProxy(sref);
                test(obj != null);

                Test.TestIntfPrx p = Test.TestIntfPrxHelper.uncheckedCast(obj);
                sref = "testController:" + helper.getTestEndpoint(1);
                obj = communicator.stringToProxy(sref);
                test(obj != null);

                Test.TestIntfControllerPrx testController = Test.TestIntfControllerPrxHelper.uncheckedCast(obj);

                var output = helper.getWriter();

                output.Write("testing async invocations...");
                output.Flush();
                {
                    Dictionary<string, string> ctx = new Dictionary<string, string>();

                    test(await p.ice_isAAsync("::Test::TestIntf"));

                    test(await p.ice_isAAsync("::Test::TestIntf", ctx));

                    await p.ice_pingAsync();

                    await p.ice_pingAsync(ctx);

                    test(await p.ice_idAsync() == "::Test::TestIntf");

                    test(await p.ice_idAsync(ctx) == "::Test::TestIntf");

                    test((await p.ice_idsAsync()).Length == 2);

                    test((await p.ice_idsAsync(ctx)).Length == 2);

                    if(!collocated)
                    {
                        test(await p.ice_getConnectionAsync() is not null);
                    }

                    await p.opAsync();
                    await p.opAsync(ctx);

                    test(await p.opWithResultAsync() == 15);

                    test(await p.opWithResultAsync(ctx) == 15);

                    try
                    {
                        await p.opWithUEAsync();
                        test(false);
                    }
                    catch (Test.TestIntfException)
                    {
                        // expected
                    }

                    try
                    {
                        await p.opWithUEAsync(ctx);
                        test(false);
                    }
                    catch (Test.TestIntfException)
                    {
                        // expected
                    }
                }
                output.WriteLine("ok");

                output.Write("testing local exceptions with async tasks... ");
                output.Flush();
                {
                    Test.TestIntfPrx indirect = Test.TestIntfPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));

                    try
                    {
                        await indirect.opAsync();
                        test(false);
                    }
                    catch (NoEndpointException)
                    {
                        // expected
                    }

                    try
                    {
                       _ = ((Test.TestIntfPrx)p.ice_oneway()).opWithResultAsync();
                        test(false);
                    }
                    catch(Ice.TwowayOnlyException)
                    {
                    }

                    //
                    // Check that CommunicatorDestroyedException is raised directly.
                    //
                    if (p.ice_getConnection() != null)
                    {
                        Ice.InitializationData initData = new Ice.InitializationData();
                        initData.properties = communicator.getProperties().ice_clone_();
                        Ice.Communicator ic = helper.initialize(initData);
                        Ice.ObjectPrx o = ic.stringToProxy(p.ToString());
                        Test.TestIntfPrx p2 = Test.TestIntfPrxHelper.checkedCast(o);
                        ic.destroy();

                        try
                        {
                            _ = p2.opAsync();
                            test(false);
                        }
                        catch (Ice.CommunicatorDestroyedException)
                        {
                            // Expected.
                        }
                    }
                }
                output.WriteLine("ok");

                output.Write("testing progress callback... ");
                output.Flush();
                {
                    {
                        // TODO: revisit the logic of this test and replace t.Wait() with await t.
                        SentCallback cb = new SentCallback();

                        Task t = p.ice_isAAsync("",
                            progress: new Progress(sentSynchronously =>
                            {
                                cb.sent(sentSynchronously);
                            }));
                        cb.check();
                        t.Wait();

                        t = p.ice_pingAsync(
                            progress: new Progress(sentSynchronously =>
                            {
                                cb.sent(sentSynchronously);
                            }));
                        cb.check();
                        t.Wait();

                        t = p.ice_idAsync(
                            progress: new Progress(sentSynchronously =>
                            {
                                cb.sent(sentSynchronously);
                            }));
                        cb.check();
                        t.Wait();

                        t = p.ice_idsAsync(
                            progress: new Progress(sentSynchronously =>
                            {
                                cb.sent(sentSynchronously);
                            }));
                        cb.check();
                        t.Wait();

                        t = p.opAsync(
                            progress: new Progress(sentSynchronously =>
                            {
                                cb.sent(sentSynchronously);
                            }));
                        cb.check();
                        t.Wait();
                    }

                    List<Task> tasks = new List<Task>();
                    byte[] seq = new byte[10024];
                    (new Random()).NextBytes(seq);
                    testController.holdAdapter();
                    try
                    {
                        Task t = null;
                        ProgressCallback cb;
                        do
                        {
                            cb = new ProgressCallback();
                            t = p.opWithPayloadAsync(seq, progress: cb);
                            tasks.Add(t);
                        }
                        while(cb.SentSynchronously);
                    }
                    finally
                    {
                        testController.resumeAdapter();
                    }

                    await Task.WhenAll(tasks);
                }
                output.WriteLine("ok");

                if(p.ice_getConnection() != null)
                {
                    output.Write("testing batch requests with connection... ");
                    output.Flush();
                    {
                        {
                            //
                            // Async task.
                            //
                            test(p.opBatchCount() == 0);
                            var b1 = Test.TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                             p.ice_getIdentity()).ice_batchOneway());
                            b1.opBatch();
                            b1.opBatch();
                            SentCallback cb = new SentCallback();
                            Task t = b1.ice_getConnection().flushBatchRequestsAsync(
                                Ice.CompressBatch.BasedOnProxy,
                                progress: new Progress(
                                    sentSynchronously =>
                                    {
                                        cb.sent(sentSynchronously);
                                    }));

                            cb.check();
                            t.Wait();
                            test(t.IsCompleted);
                            test(p.waitForBatch(2));
                        }

                        {
                            //
                            // Async task exception.
                            //
                            test(p.opBatchCount() == 0);
                            var b1 = Test.TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                                 p.ice_getIdentity()).ice_batchOneway());
                            b1.opBatch();
                            b1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                            Task t = b1.ice_getConnection().flushBatchRequestsAsync(
                                Ice.CompressBatch.BasedOnProxy,
                                progress: new Progress(
                                    sentSynchronously =>
                                    {
                                        test(false);
                                    }));
                            try
                            {
                                await t;
                                test(false);
                            }
                            catch
                            {
                            }
                            test(p.opBatchCount() == 0);
                        }
                    }
                    output.WriteLine("ok");
                }

                if(p.ice_getConnection() != null)
                {
                    output.Write("testing batch requests with communicator... ");
                    output.Flush();
                    {
                        {
                            //
                            // Async task - 1 connection.
                            //
                            test(p.opBatchCount() == 0);
                            var b1 = Test.TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                                 p.ice_getIdentity()).ice_batchOneway());
                            b1.opBatch();
                            b1.opBatch();

                            SentCallback cb = new SentCallback();
                            Task t = communicator.flushBatchRequestsAsync(
                                Ice.CompressBatch.BasedOnProxy,
                                progress: new Progress(
                                    sentSynchronously =>
                                    {
                                        cb.sent(sentSynchronously);
                                    }));
                            cb.check();
                            t.Wait();
                            test(t.IsCompleted);
                            test(p.waitForBatch(2));
                        }

                        {
                            //
                            // Async task exception - 1 connection.
                            //
                            test(p.opBatchCount() == 0);
                            var b1 = Test.TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                                 p.ice_getIdentity()).ice_batchOneway());
                            b1.opBatch();
                            b1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                            SentCallback cb = new SentCallback();
                            Task t = communicator.flushBatchRequestsAsync(
                                Ice.CompressBatch.BasedOnProxy,
                                progress: new Progress(
                                    sentSynchronously =>
                                    {
                                        cb.sent(sentSynchronously);
                                    }));
                            cb.check(); // Exceptions are ignored!
                            t.Wait();
                            test(t.IsCompleted);
                            test(p.opBatchCount() == 0);
                        }

                        {
                            //
                            // Async task - 2 connections.
                            //
                            test(p.opBatchCount() == 0);
                            var b1 = Test.TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                                 p.ice_getIdentity()).ice_batchOneway());
                            var b2 = Test.TestIntfPrxHelper.uncheckedCast(
                                p.ice_connectionId("2").ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway());

                            b2.ice_getConnection(); // Ensure connection is established.
                            b1.opBatch();
                            b1.opBatch();
                            b2.opBatch();
                            b2.opBatch();

                            SentCallback cb = new SentCallback();
                            Task t = communicator.flushBatchRequestsAsync(
                                Ice.CompressBatch.BasedOnProxy,
                                new Progress(sentSynchronously =>
                                    {
                                        cb.sent(sentSynchronously);
                                    }));
                            cb.check();
                            t.Wait();
                            test(t.IsCompleted);
                            test(p.waitForBatch(4));
                        }

                        {
                            //
                            // AsyncResult exception - 2 connections - 1 failure.
                            //
                            // All connections should be flushed even if there are failures on some connections.
                            // Exceptions should not be reported.
                            //
                            test(p.opBatchCount() == 0);
                            var b1 = Test.TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                                 p.ice_getIdentity()).ice_batchOneway());
                            var b2 = Test.TestIntfPrxHelper.uncheckedCast(
                                p.ice_connectionId("2").ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway());
                            b2.ice_getConnection(); // Ensure connection is established.
                            b1.opBatch();
                            b2.opBatch();
                            b1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                            SentCallback cb = new SentCallback();
                            Task t = communicator.flushBatchRequestsAsync(
                                Ice.CompressBatch.BasedOnProxy,
                                new Progress(
                                    sentSynchronously =>
                                    {
                                        cb.sent(sentSynchronously);
                                    }));
                            cb.check(); // Exceptions are ignored!
                            t.Wait();
                            test(t.IsCompleted);
                            test(p.waitForBatch(1));
                        }

                        {
                            //
                            // Async task exception - 2 connections - 2 failures.
                            //
                            // The sent callback should be invoked even if all connections fail.
                            //
                            test(p.opBatchCount() == 0);
                            var b1 = Test.TestIntfPrxHelper.uncheckedCast(p.ice_getConnection().createProxy(
                                                                                 p.ice_getIdentity()).ice_batchOneway());
                            var b2 = Test.TestIntfPrxHelper.uncheckedCast(
                                p.ice_connectionId("2").ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway());
                            b2.ice_getConnection(); // Ensure connection is established.
                            b1.opBatch();
                            b2.opBatch();
                            b1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                            b2.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                            SentCallback cb = new SentCallback();
                            Task t = communicator.flushBatchRequestsAsync(
                                Ice.CompressBatch.BasedOnProxy,
                                new Progress(
                                    sentSynchronously =>
                                    {
                                        cb.sent(sentSynchronously);
                                    }));
                            cb.check(); // Exceptions are ignored!
                            t.Wait();
                            test(t.IsCompleted);
                            test(p.opBatchCount() == 0);
                        }
                    }
                    output.WriteLine("ok");
                }

                if(p.ice_getConnection() != null)
                {
                    output.Write("testing async Task cancellation... ");
                    output.Flush();
                    {
                        var cs1 = new CancellationTokenSource();
                        var cs2 = new CancellationTokenSource();
                        var cs3 = new CancellationTokenSource();
                        Task t1;
                        Task t2;
                        Task t3;
                        try
                        {
                            testController.holdAdapter();
                            ProgressCallback cb = null;
                            byte[] seq = new byte[10024];
                            for(int i = 0; i < 200; ++i) // 2MB
                            {
                                cb = new ProgressCallback();
                                _ = p.opWithPayloadAsync(seq, progress: cb);
                            }

                            test(!cb.Sent);

                            t1 = p.ice_pingAsync(cancel: cs1.Token);
                            t2 = p.ice_pingAsync(cancel: cs2.Token);
                            cs3.Cancel();
                            t3 = p.ice_pingAsync(cancel: cs3.Token);
                            cs1.Cancel();
                            cs2.Cancel();
                            try
                            {
                                t1.Wait();
                                test(false);
                            }
                            catch(AggregateException ae)
                            {
                                ae.Handle(ex =>
                                {
                                    return ex is Ice.InvocationCanceledException;
                                });
                            }
                            try
                            {
                                t2.Wait();
                                test(false);
                            }
                            catch(AggregateException ae)
                            {
                                ae.Handle(ex =>
                                {
                                    return ex is Ice.InvocationCanceledException;
                                });
                            }

                            try
                            {
                                t3.Wait();
                                test(false);
                            }
                            catch(AggregateException ae)
                            {
                                ae.Handle(ex =>
                                {
                                    return ex is Ice.InvocationCanceledException;
                                });
                            }

                        }
                        finally
                        {
                            testController.resumeAdapter();
                            p.ice_ping();
                        }
                    }
                    output.WriteLine("ok");
                }

                if(p.ice_getConnection() != null && p.supportsAMD())
                {
                    output.Write("testing graceful close connection with wait... ");
                    output.Flush();
                    {
                        //
                        // Local case: begin a request, close the connection gracefully, and make sure it waits
                        // for the request to complete.
                        //
                        Ice.Connection con = p.ice_getConnection();
                        CallbackBase cb = new CallbackBase();
                        con.setCloseCallback(_ =>
                            {
                                cb.called();
                            });
                        Task t = p.sleepAsync(100);
                        con.close(Ice.ConnectionClose.GracefullyWithWait);
                        t.Wait(); // Should complete successfully.
                        cb.check();
                    }
                    {
                        //
                        // Remote case.
                        //
                        byte[] seq = new byte[1024 * 10];

                        //
                        // Send multiple opWithPayload, followed by a close and followed by multiple opWithPaylod.
                        // The goal is to make sure that none of the opWithPayload fail even if the server closes
                        // the connection gracefully in between.
                        //
                        int maxQueue = 2;
                        bool done = false;
                        while(!done && maxQueue < 50)
                        {
                            done = true;
                            p.ice_ping();
                            List<Task> results = new List<Task>();
                            for(int i = 0; i < maxQueue; ++i)
                            {
                                results.Add(p.opWithPayloadAsync(seq));
                            }

                            ProgressCallback cb = new ProgressCallback();
                            _ = p.closeAsync(Test.CloseMode.GracefullyWithWait, progress: cb);

                            if(!cb.SentSynchronously)
                            {
                                for(int i = 0; i < maxQueue; i++)
                                {
                                    cb = new ProgressCallback();
                                    Task t = p.opWithPayloadAsync(seq, progress: cb);
                                    results.Add(t);
                                    if(cb.SentSynchronously)
                                    {
                                        done = false;
                                        maxQueue *= 2;
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                maxQueue *= 2;
                                done = false;
                            }
                            foreach(Task q in results)
                            {
                                q.Wait();
                            }
                        }
                    }
                    output.WriteLine("ok");

                    output.Write("testing graceful close connection without wait... ");
                    output.Flush();
                    {
                        //
                        // Local case: start an operation and then close the connection gracefully on the client side
                        // without waiting for the pending invocation to complete. There will be no retry and we expect the
                        // invocation to fail with ConnectionManuallyClosedException.
                        //
                        p =(Test.TestIntfPrx)p.ice_connectionId("CloseGracefully"); // Start with a new connection.
                        Ice.Connection con = p.ice_getConnection();
                        CallbackBase cb = new CallbackBase();
                        Task t = p.startDispatchAsync(
                            progress: new Progress(sentSynchronously =>
                            {
                                cb.called();
                            }));
                        cb.check(); // Ensure the request was sent before we close the connection.
                        con.close(Ice.ConnectionClose.Gracefully);
                        try
                        {
                            t.Wait();
                            test(false);
                        }
                        catch(System.AggregateException ex)
                        {
                            test(ex.InnerException is Ice.ConnectionManuallyClosedException);
                            test((ex.InnerException as Ice.ConnectionManuallyClosedException).graceful);
                        }
                        p.finishDispatch();

                        //
                        // Remote case: the server closes the connection gracefully, which means the connection
                        // will not be closed until all pending dispatched requests have completed.
                        //
                        con = p.ice_getConnection();
                        cb = new CallbackBase();
                        con.setCloseCallback(_ =>
                            {
                                cb.called();
                            });
                        t = p.sleepAsync(100);
                        p.close(Test.CloseMode.Gracefully); // Close is delayed until sleep completes.
                        cb.check();
                        t.Wait();
                    }
                    output.WriteLine("ok");

                    output.Write("testing forceful close connection... ");
                    output.Flush();
                    {
                        //
                        // Local case: start an operation and then close the connection forcefully on the client side.
                        // There will be no retry and we expect the invocation to fail with ConnectionManuallyClosedException.
                        //
                        p.ice_ping();
                        Ice.Connection con = p.ice_getConnection();
                        CallbackBase cb = new CallbackBase();
                        Task t = p.startDispatchAsync(
                            progress: new Progress(sentSynchronously =>
                            {
                                cb.called();
                            }));
                        cb.check(); // Ensure the request was sent before we close the connection.
                        con.close(Ice.ConnectionClose.Forcefully);
                        try
                        {
                            t.Wait();
                            test(false);
                        }
                        catch(AggregateException ex)
                        {
                            test(ex.InnerException is Ice.ConnectionManuallyClosedException);
                            test(!(ex.InnerException as Ice.ConnectionManuallyClosedException).graceful);
                        }
                        p.finishDispatch();

                        //
                        // Remote case: the server closes the connection forcefully. This causes the request to fail
                        // with a ConnectionLostException. Since the close() operation is not idempotent, the client
                        // will not retry.
                        //
                        try
                        {
                            p.close(Test.CloseMode.Forcefully);
                            test(false);
                        }
                        catch(Ice.ConnectionLostException)
                        {
                            // Expected.
                        }
                    }
                    output.WriteLine("ok");
                }

                output.Write("testing ice_scheduler... ");
                output.Flush();
                {
                    p.ice_pingAsync().ContinueWith(
                       (t) =>
                        {
                            test(Thread.CurrentThread.Name == null ||
                                 !Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Client"));
                        }).Wait();

                    p.ice_pingAsync().ContinueWith(
                       (t) =>
                        {
                            test(Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Client"));
                        }, p.ice_scheduler()).Wait();

                    {
                        TaskCompletionSource<int> s1 = new TaskCompletionSource<int>();
                        TaskCompletionSource<int> s2 = new TaskCompletionSource<int>();
                        Task t1 = s1.Task;
                        Task t2 = s2.Task;
                        Task t3 = null;
                        Task t4 = null;
                        p.ice_pingAsync().ContinueWith(
                           (t) =>
                            {
                                test(Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Client"));
                                    //
                                    // t1 Continuation run in the thread that completes it.
                                    //
                                    var id = Thread.CurrentThread.ManagedThreadId;
                                t3 = t1.ContinueWith(prev =>
                                    {
                                        test(id == Thread.CurrentThread.ManagedThreadId);
                                    },
                                    CancellationToken.None,
                                    TaskContinuationOptions.ExecuteSynchronously,
                                    p.ice_scheduler());
                                s1.SetResult(1);

                                    //
                                    // t2 completed from the main thread
                                    //
                                    t4 = t2.ContinueWith(prev =>
                                                {
                                                    test(id != Thread.CurrentThread.ManagedThreadId);
                                                    test(Thread.CurrentThread.Name == null ||
                                                         !Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Client"));
                                                },
                                                CancellationToken.None,
                                                TaskContinuationOptions.ExecuteSynchronously,
                                                p.ice_scheduler());
                            }, p.ice_scheduler()).Wait();
                        s2.SetResult(1);
                        Task.WaitAll(t1, t2, t3, t4);
                    }

                    if(!collocated)
                    {
                        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("");
                        PingReplyI replyI = new PingReplyI();
                        var reply = Test.PingReplyPrxHelper.uncheckedCast(adapter.addWithUUID(replyI));
                        adapter.activate();

                        p.ice_getConnection().setAdapter(adapter);
                        p.pingBiDir(reply);
                        test(replyI.checkReceived());
                        adapter.destroy();
                    }
                }
                output.WriteLine("ok");

                output.Write("testing result struct... ");
                output.Flush();
                {
                    var q = Test.Outer.Inner.TestIntfPrxHelper.uncheckedCast(
                        communicator.stringToProxy("test2:" + helper.getTestEndpoint(0)));

                    var r = await q.opAsync(1);
                    test(r.returnValue == 1);
                    test(r.j == 1);
                }
                output.WriteLine("ok");

                p.shutdown();
            }
        }
    }
}
