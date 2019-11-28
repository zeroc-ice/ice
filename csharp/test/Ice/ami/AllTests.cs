//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using Ice.ami.Test;

namespace Ice
{
    namespace ami
    {
        public class AllTests : global::Test.AllTests
        {
            public class PingReplyI : Test.PingReply
            {
                public void reply(Current current)
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

            public class ProgresCallback : IProgress<bool>
            {
                public bool Sent
                {
                    get
                    {
                        lock (this)
                        {
                            return _sent;
                        }
                    }
                    set
                    {
                        lock (this)
                        {
                            _sent = value;
                        }
                    }
                }

                public bool SentSynchronously
                {
                    get
                    {
                        lock (this)
                        {
                            return _sentSynchronously;
                        }
                    }
                    set
                    {
                        lock (this)
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
                    lock (this)
                    {
                        while (!_called)
                        {
                            Monitor.Wait(this);
                        }
                        _called = false;
                    }
                }

                public virtual void called()
                {
                    lock (this)
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

            private class FlushCallback : CallbackBase
            {
                public FlushCallback()
                {
                    _thread = Thread.CurrentThread;
                }

                public void
                sent(bool sentSynchronously)
                {
                    test(sentSynchronously && _thread == Thread.CurrentThread ||
                         !sentSynchronously && _thread != Thread.CurrentThread);
                    called();
                }

                Thread _thread;
            }

            public static void allTests(global::Test.TestHelper helper, bool collocated)
            {
                Communicator communicator = helper.communicator();

                var p = TestIntfPrx.Parse($"test:{helper.getTestEndpoint(0)}", communicator);
                var testController = TestIntfControllerPrx.Parse($"testController:{helper.getTestEndpoint(1)}", communicator);

                var output = helper.getWriter();

                output.Write("testing async invocation...");
                output.Flush();
                {
                    Dictionary<string, string> ctx = new Dictionary<string, string>();

                    test(p.IceIsAAsync("::Test::TestIntf").Result);
                    test(p.IceIsAAsync("::Test::TestIntf", ctx).Result);

                    p.IcePingAsync().Wait();
                    p.IcePingAsync(ctx).Wait();

                    test(p.IceIdAsync().Result.Equals("::Test::TestIntf"));
                    test(p.IceIdAsync(ctx).Result.Equals("::Test::TestIntf"));

                    test(p.IceIdsAsync().Result.Length == 2);
                    test(p.IceIdsAsync(ctx).Result.Length == 2);

                    if (!collocated)
                    {
                        test(p.GetConnectionAsync().Result != null);
                    }

                    p.opAsync().Wait();
                    p.opAsync(ctx).Wait();

                    test(p.opWithResultAsync().Result == 15);
                    test(p.opWithResultAsync(ctx).Result == 15);

                    try
                    {
                        p.opWithUEAsync().Wait();
                        test(false);
                    }
                    catch (AggregateException ae)
                    {
                        ae.Handle(ex => ex is Test.TestIntfException);
                    }

                    try
                    {
                        p.opWithUEAsync(ctx).Wait();
                        test(false);
                    }
                    catch (AggregateException ae)
                    {
                        ae.Handle(ex => ex is Test.TestIntfException);
                    }
                }
                output.WriteLine("ok");

                output.Write("testing async/await...");
                output.Flush();
                {
                    Task.Run(async () =>
                        {
                            Dictionary<string, string> ctx = new Dictionary<string, string>();

                            test(await p.IceIsAAsync("::Test::TestIntf"));
                            test(await p.IceIsAAsync("::Test::TestIntf", ctx));

                            await p.IcePingAsync();
                            await p.IcePingAsync(ctx);

                            var id = await p.IceIdAsync();
                            test(id.Equals("::Test::TestIntf"));
                            id = await p.IceIdAsync(ctx);
                            test(id.Equals("::Test::TestIntf"));

                            var ids = await p.IceIdsAsync();
                            test(ids.Length == 2);
                            ids = await p.IceIdsAsync(ctx);
                            test(ids.Length == 2);

                            if (!collocated)
                            {
                                var conn = await p.GetConnectionAsync();
                                test(conn != null);
                            }

                            await p.opAsync();
                            await p.opAsync(ctx);

                            var result = await p.opWithResultAsync();
                            test(result == 15);
                            result = await p.opWithResultAsync(ctx);
                            test(result == 15);

                            try
                            {
                                await p.opWithUEAsync();
                                test(false);
                            }
                            catch (System.Exception ex)
                            {
                                test(ex is Test.TestIntfException);
                            }

                            try
                            {
                                await p.opWithUEAsync(ctx);
                                test(false);
                            }
                            catch (System.Exception ex)
                            {
                                test(ex is Test.TestIntfException);
                            }
                        }).Wait();
                }
                output.WriteLine("ok");

                output.Write("testing async continuations...");
                output.Flush();
                {
                    Dictionary<string, string> ctx = new Dictionary<string, string>();

                    p.IceIsAAsync("::Test::TestIntf").ContinueWith(previous =>
                        {
                            test(previous.Result);
                        }).Wait();

                    p.IceIsAAsync("::Test::TestIntf", ctx).ContinueWith(previous =>
                        {
                            test(previous.Result);
                        }).Wait();

                    p.IcePingAsync().ContinueWith(previous =>
                        {
                            previous.Wait();
                        }).Wait();

                    p.IcePingAsync(ctx).ContinueWith(previous =>
                        {
                            previous.Wait();
                        }).Wait();

                    p.IceIdAsync().ContinueWith(previous =>
                        {
                            test(previous.Result.Equals("::Test::TestIntf"));
                        }).Wait();

                    p.IceIdAsync(ctx).ContinueWith(previous =>
                        {
                            test(previous.Result.Equals("::Test::TestIntf"));
                        }).Wait();

                    p.IceIdsAsync().ContinueWith(previous =>
                        {
                            test(previous.Result.Length == 2);
                        }).Wait();

                    p.IceIdsAsync(ctx).ContinueWith(previous =>
                        {
                            test(previous.Result.Length == 2);
                        }).Wait();

                    if (!collocated)
                    {
                        p.GetConnectionAsync().ContinueWith(previous =>
                            {
                                test(previous.Result != null);
                            }).Wait();
                    }

                    p.opAsync().ContinueWith(previous => previous.Wait()).Wait();
                    p.opAsync(ctx).ContinueWith(previous => previous.Wait()).Wait();

                    p.opWithResultAsync().ContinueWith(previous =>
                        {
                            test(previous.Result == 15);
                        }).Wait();

                    p.opWithResultAsync(ctx).ContinueWith(previous =>
                        {
                            test(previous.Result == 15);
                        }).Wait();

                    p.opWithUEAsync().ContinueWith(previous =>
                        {
                            try
                            {
                                previous.Wait();
                            }
                            catch (AggregateException ae)
                            {
                                ae.Handle(ex => ex is Test.TestIntfException);
                            }
                        }).Wait();

                    p.opWithUEAsync(ctx).ContinueWith(previous =>
                        {
                            try
                            {
                                previous.Wait();
                            }
                            catch (AggregateException ae)
                            {
                                ae.Handle(ex => ex is Test.TestIntfException);
                            }
                        }).Wait();
                }
                output.WriteLine("ok");

                output.Write("testing local exceptions with async tasks... ");
                output.Flush();
                {
                    TestIntfPrx indirect = p.Clone(adapterId: "dummy");

                    try
                    {
                        indirect.opAsync().Wait();
                        test(false);
                    }
                    catch (AggregateException ae)
                    {
                        ae.Handle((ex) =>
                        {
                            return ex is NoEndpointException;
                        });
                    }

                    try
                    {
                        p.Clone(oneway: true).opWithResultAsync();
                        test(false);
                    }
                    catch (TwowayOnlyException)
                    {
                    }

                    //
                    // Check that CommunicatorDestroyedException is raised directly.
                    //
                    if (p.GetConnection() != null)
                    {
                        InitializationData initData = new InitializationData();
                        initData.properties = communicator.getProperties().ice_clone_();
                        Communicator ic = helper.initialize(initData);
                        IObjectPrx o = IObjectPrx.Parse(p.ToString(), ic);
                        TestIntfPrx p2 = Test.TestIntfPrx.CheckedCast(o);
                        ic.destroy();

                        try
                        {
                            p2.opAsync();
                            test(false);
                        }
                        catch (CommunicatorDestroyedException)
                        {
                            // Expected.
                        }
                    }
                }
                output.WriteLine("ok");

                output.Write("testing exception with async task... ");
                output.Flush();
                {
                    TestIntfPrx i = p.Clone(adapterId: "dummy");

                    try
                    {
                        i.IceIsAAsync("::Test::TestIntf").Wait();
                        test(false);
                    }
                    catch (AggregateException)
                    {
                    }

                    try
                    {
                        i.opAsync().Wait();
                        test(false);
                    }
                    catch (AggregateException)
                    {
                    }

                    try
                    {
                        i.opWithResultAsync().Wait();
                        test(false);
                    }
                    catch (AggregateException)
                    {
                    }

                    try
                    {
                        i.opWithUEAsync().Wait();
                        test(false);
                    }
                    catch (AggregateException)
                    {
                    }

                    // Ensures no exception is called when response is received
                    test(p.IceIsAAsync("::Test::TestIntf").Result);
                    p.opAsync().Wait();
                    p.opWithResultAsync().Wait();

                    // If response is a user exception, it should be received.
                    try
                    {
                        p.opWithUEAsync().Wait();
                        test(false);
                    }
                    catch (AggregateException ae)
                    {
                        ae.Handle((ex) =>
                        {
                            return ex is Test.TestIntfException;
                        });
                    }
                }
                output.WriteLine("ok");

                output.Write("testing progress callback... ");
                output.Flush();
                {
                    {
                        SentCallback cb = new SentCallback();

                        Task t = p.IceIsAAsync("",
                            progress: new Progress(sentSynchronously =>
                            {
                                cb.sent(sentSynchronously);
                            }));
                        cb.check();
                        t.Wait();

                        t = p.IcePingAsync(
                            progress: new Progress(sentSynchronously =>
                            {
                                cb.sent(sentSynchronously);
                            }));
                        cb.check();
                        t.Wait();

                        t = p.IceIdAsync(
                            progress: new Progress(sentSynchronously =>
                            {
                                cb.sent(sentSynchronously);
                            }));
                        cb.check();
                        t.Wait();

                        t = p.IceIdsAsync(
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
                        ProgresCallback cb;
                        do
                        {
                            cb = new ProgresCallback();
                            t = p.opWithPayloadAsync(seq, progress: cb);
                            tasks.Add(t);
                        }
                        while (cb.SentSynchronously);
                    }
                    finally
                    {
                        testController.resumeAdapter();
                    }
                    foreach (Task t in tasks)
                    {
                        t.Wait();
                    }
                }
                output.WriteLine("ok");
                output.Write("testing async/await... ");
                output.Flush();
                Func<Task> task = async () =>
                {
                    try
                    {
                        await p.opAsync();

                        var r = await p.opWithResultAsync();
                        test(r == 15);

                        try
                        {
                            await p.opWithUEAsync();
                        }
                        catch (Test.TestIntfException)
                        {
                        }

                        // Operations implemented with amd and async.
                        await p.opAsyncDispatchAsync();

                        r = await p.opWithResultAsyncDispatchAsync();
                        test(r == 15);

                        try
                        {
                            await p.opWithUEAsyncDispatchAsync();
                            test(false);
                        }
                        catch (Test.TestIntfException)
                        {
                        }
                    }
                    catch (OperationNotExistException)
                    {
                        // Expected with cross testing, this opXxxAsyncDispatch methods are C# only.
                    }
                };
                task().Wait();
                output.WriteLine("ok");

                if (p.GetConnection() != null)
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
                            ProgresCallback cb = null;
                            byte[] seq = new byte[10024];
                            for (int i = 0; i < 200; ++i) // 2MB
                            {
                                cb = new ProgresCallback();
                                p.opWithPayloadAsync(seq, progress: cb);
                            }

                            test(!cb.Sent);

                            t1 = p.IcePingAsync(cancel: cs1.Token);
                            t2 = p.IcePingAsync(cancel: cs2.Token);
                            cs3.Cancel();
                            t3 = p.IcePingAsync(cancel: cs3.Token);
                            cs1.Cancel();
                            cs2.Cancel();
                            try
                            {
                                t1.Wait();
                                test(false);
                            }
                            catch (AggregateException ae)
                            {
                                ae.Handle(ex =>
                                {
                                    return ex is InvocationCanceledException;
                                });
                            }
                            try
                            {
                                t2.Wait();
                                test(false);
                            }
                            catch (AggregateException ae)
                            {
                                ae.Handle(ex =>
                                {
                                    return ex is InvocationCanceledException;
                                });
                            }

                            try
                            {
                                t3.Wait();
                                test(false);
                            }
                            catch (AggregateException ae)
                            {
                                ae.Handle(ex =>
                                {
                                    return ex is InvocationCanceledException;
                                });
                            }

                        }
                        finally
                        {
                            testController.resumeAdapter();
                            p.IcePing();
                        }
                    }
                    output.WriteLine("ok");
                }

                if (p.GetConnection() != null && p.supportsAMD())
                {
                    output.Write("testing graceful close connection with wait... ");
                    output.Flush();
                    {
                        //
                        // Local case: begin a request, close the connection gracefully, and make sure it waits
                        // for the request to complete.
                        //
                        Connection con = p.GetConnection();
                        CallbackBase cb = new CallbackBase();
                        con.setCloseCallback(_ =>
                            {
                                cb.called();
                            });
                        Task t = p.sleepAsync(100);
                        con.close(ConnectionClose.GracefullyWithWait);
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
                        while (!done && maxQueue < 50)
                        {
                            done = true;
                            p.IcePing();
                            List<Task> results = new List<Task>();
                            for (int i = 0; i < maxQueue; ++i)
                            {
                                results.Add(p.opWithPayloadAsync(seq));
                            }

                            ProgresCallback cb = new ProgresCallback();
                            p.closeAsync(Test.CloseMode.GracefullyWithWait, progress: cb);

                            if (!cb.SentSynchronously)
                            {
                                for (int i = 0; i < maxQueue; i++)
                                {
                                    cb = new ProgresCallback();
                                    Task t = p.opWithPayloadAsync(seq, progress: cb);
                                    results.Add(t);
                                    if (cb.SentSynchronously)
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
                            foreach (Task q in results)
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
                        p = p.Clone(connectionId: "CloseGracefully"); // Start with a new connection.
                        Connection con = p.GetConnection();
                        CallbackBase cb = new CallbackBase();
                        Task t = p.startDispatchAsync(
                            progress: new Progress(sentSynchronously =>
                            {
                                cb.called();
                            }));
                        cb.check(); // Ensure the request was sent before we close the connection.
                        con.close(ConnectionClose.Gracefully);
                        try
                        {
                            t.Wait();
                            test(false);
                        }
                        catch (System.AggregateException ex)
                        {
                            test(ex.InnerException is ConnectionManuallyClosedException);
                            test((ex.InnerException as ConnectionManuallyClosedException).graceful);
                        }
                        p.finishDispatch();

                        //
                        // Remote case: the server closes the connection gracefully, which means the connection
                        // will not be closed until all pending dispatched requests have completed.
                        //
                        con = p.GetConnection();
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
                        p.IcePing();
                        Connection con = p.GetConnection();
                        CallbackBase cb = new CallbackBase();
                        Task t = p.startDispatchAsync(
                            progress: new Progress(sentSynchronously =>
                            {
                                cb.called();
                            }));
                        cb.check(); // Ensure the request was sent before we close the connection.
                        con.close(ConnectionClose.Forcefully);
                        try
                        {
                            t.Wait();
                            test(false);
                        }
                        catch (AggregateException ex)
                        {
                            test(ex.InnerException is ConnectionManuallyClosedException);
                            test(!(ex.InnerException as ConnectionManuallyClosedException).graceful);
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
                        catch (ConnectionLostException)
                        {
                            // Expected.
                        }
                    }
                    output.WriteLine("ok");
                }

                output.Write("testing ice_scheduler... ");
                output.Flush();
                {
                    p.IcePingAsync().ContinueWith(
                       (t) =>
                        {
                            test(Thread.CurrentThread.Name == null ||
                                 !Thread.CurrentThread.Name.Contains("ThreadPool.Client"));
                        }).Wait();

                    p.IcePingAsync().ContinueWith(
                       (t) =>
                        {
                            test(Thread.CurrentThread.Name.Contains("ThreadPool.Client"));
                        }, p.Scheduler).Wait();

                    {
                        TaskCompletionSource<int> s1 = new TaskCompletionSource<int>();
                        TaskCompletionSource<int> s2 = new TaskCompletionSource<int>();
                        Task t1 = s1.Task;
                        Task t2 = s2.Task;
                        Task t3 = null;
                        Task t4 = null;
                        p.IcePingAsync().ContinueWith(
                           (t) =>
                            {
                                test(Thread.CurrentThread.Name.Contains("ThreadPool.Client"));
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
                                    p.Scheduler);
                                s1.SetResult(1);

                                //
                                // t2 completed from the main thread
                                //
                                t4 = t2.ContinueWith(prev =>
                                            {
                                                test(id != Thread.CurrentThread.ManagedThreadId);
                                                test(Thread.CurrentThread.Name == null ||
                                                     !Thread.CurrentThread.Name.Contains("ThreadPool.Client"));
                                            },
                                            CancellationToken.None,
                                            TaskContinuationOptions.ExecuteSynchronously,
                                            p.Scheduler);
                            }, p.Scheduler).Wait();
                        s2.SetResult(1);
                        Task.WaitAll(t1, t2, t3, t4);
                    }

                    if (!collocated)
                    {
                        ObjectAdapter adapter = communicator.createObjectAdapter("");
                        PingReplyI replyI = new PingReplyI();
                        var reply = adapter.Add(replyI);
                        adapter.Activate();

                        p.GetConnection().setAdapter(adapter);
                        p.pingBiDir(reply);
                        test(replyI.checkReceived());
                        adapter.Destroy();
                    }
                }
                output.WriteLine("ok");

                output.Write("testing result struct... ");
                output.Flush();
                {
                    var q = Test.Outer.Inner.TestIntfPrx.Parse($"test2:{helper.getTestEndpoint(0)}", communicator);
                    q.opAsync(1).ContinueWith(t =>
                        {
                            var r = t.Result;
                            test(r.returnValue == 1);
                            test(r.j == 1);
                        }).Wait();
                }
                output.WriteLine("ok");

                p.shutdown();
            }
        }
    }
}
