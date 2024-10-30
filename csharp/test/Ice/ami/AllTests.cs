// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Runtime.CompilerServices;

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

            public class ProgressCallback : IProgress<bool>
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

            private enum ThrowType { LocalException, UserException, OtherException };

            private class Thrower
            {
                public Thrower(ThrowType t)
                {
                    _t = t;
                }

                public void op() => throwEx();

                public void noOp()
                {
                }

                public void ex(Exception ex) => throwEx();

                public void sent(bool ss) => throwEx();

                private void
                throwEx()
                {
                    switch (_t)
                    {
                        case ThrowType.LocalException:
                            {
                                throw new Ice.ObjectNotExistException();
                            }
                        case ThrowType.UserException:
                            {
                                throw new Test.TestIntfException();
                            }
                        case ThrowType.OtherException:
                            {
                                throw new System.Exception();
                            }
                        default:
                            {
                                Debug.Assert(false);
                                break;
                            }
                    }
                }

                private ThrowType _t;
            }

            public static async Task allTestsAsync(global::Test.TestHelper helper, bool collocated)
            {
                Ice.Communicator communicator = helper.communicator();

                string sref = "test:" + helper.getTestEndpoint(0);
                var p = Test.TestIntfPrxHelper.createProxy(communicator, sref);

                sref = "testController:" + helper.getTestEndpoint(1);
                var testController = Test.TestIntfControllerPrxHelper.createProxy(communicator, sref);

                var output = helper.getWriter();

                output.Write("testing async/await...");
                output.Flush();
                {
                    Dictionary<string, string> ctx = new Dictionary<string, string>();

                    test(await p.ice_isAAsync("::Test::TestIntf"));
                    test(await p.ice_isAAsync("::Test::TestIntf", ctx));

                    await p.ice_pingAsync();
                    await p.ice_pingAsync(ctx);

                    var id = await p.ice_idAsync();
                    test(id == "::Test::TestIntf");
                    id = await p.ice_idAsync(ctx);
                    test(id == "::Test::TestIntf");

                    var ids = await p.ice_idsAsync();
                    test(ids.Length == 2);
                    ids = await p.ice_idsAsync(ctx);
                    test(ids.Length == 2);

                    if (!collocated)
                    {
                        var conn = await p.ice_getConnectionAsync();
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
                    }

                    try
                    {
                        _ = ((Test.TestIntfPrx)p.ice_oneway()).opWithResultAsync();
                        test(false);
                    }
                    catch (TwowayOnlyException)
                    {
                    }

                    //
                    // Check that CommunicatorDestroyedException is raised directly.
                    //
                    if (p.ice_getConnection() != null)
                    {
                        var initData = new InitializationData();
                        initData.properties = communicator.getProperties().Clone();
                        Communicator ic = helper.initialize(initData);
                        Test.TestIntfPrx p2 = Test.TestIntfPrxHelper.createProxy(ic, p.ToString());
                        ic.destroy();

                        try
                        {
                            _ = p2.opAsync();
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
                    Test.TestIntfPrx i = Test.TestIntfPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));

                    try
                    {
                        await i.ice_isAAsync("::Test::TestIntf");
                        test(false);
                    }
                    catch (NoEndpointException)
                    {
                    }

                    try
                    {
                        await i.opAsync();
                        test(false);
                    }
                    catch (NoEndpointException)
                    {
                    }

                    try
                    {
                        await i.opWithResultAsync();
                        test(false);
                    }
                    catch (NoEndpointException)
                    {
                    }

                    try
                    {
                        await i.opWithUEAsync();
                        test(false);
                    }
                    catch (NoEndpointException)
                    {
                    }

                    // Ensures no exception is called when response is received
                    test(await p.ice_isAAsync("::Test::TestIntf"));
                    await p.opAsync();
                    await p.opWithResultAsync();

                    // If response is a user exception, it should be received.
                    try
                    {
                        await p.opWithUEAsync();
                        test(false);
                    }
                    catch (Test.TestIntfException)
                    {
                    }
                }
                output.WriteLine("ok");

                output.Write("testing progress callback... ");
                output.Flush();
                {
                    {
                        var tcs = new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously);

                        Task t = p.ice_isAAsync(
                            "",
                            progress: new Progress<bool>(_ => tcs.SetResult()));
                        await tcs.Task;
                        await t;

                        tcs = new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously);
                        t = p.ice_pingAsync(
                            progress: new Progress<bool>(_ => tcs.SetResult()));
                        await tcs.Task;
                        await t;

                        tcs = new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously);
                        t = p.ice_idAsync(
                            progress: new Progress<bool>(_ => tcs.SetResult()));
                        await tcs.Task;
                        await t;

                        tcs = new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously);
                        t = p.ice_idsAsync(
                            progress: new Progress<bool>(_ => tcs.SetResult()));
                        await tcs.Task;
                        await t;

                        tcs = new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously);
                        t = p.opAsync(
                            progress: new Progress<bool>(_ => tcs.SetResult()));
                        await tcs.Task;
                        await t;
                    }

                    var tasks = new List<Task>();
                    byte[] seq = new byte[10024];
                    (new Random()).NextBytes(seq);
                    testController.holdAdapter();
                    try
                    {
                        Task t = null;
                        bool sentSynchronously;
                        do
                        {
                            sentSynchronously = false;
                            t = p.opWithPayloadAsync(
                                seq,
                                progress: new Progress<bool>(value => sentSynchronously = value));
                            tasks.Add(t);
                        }
                        while (sentSynchronously);
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

                output.Write("testing batch requests with proxy and async tasks... ");
                output.Flush();
                {
                    {
                        var tcs = new TaskCompletionSource();
                        Task t = p.ice_batchOneway().ice_flushBatchRequestsAsync(
                            progress: new Progress<bool>(_ => tcs.SetResult()));
                        await tcs.Task;
                        await t;
                    }

                    {
                        test(p.opBatchCount() == 0);
                        var b1 = (Test.TestIntfPrx)p.ice_batchOneway();
                        b1.opBatch();
                        var bf = b1.opBatchAsync();
                        test(bf.IsCompleted);
                        var tcs = new TaskCompletionSource();
                        Task t = b1.ice_flushBatchRequestsAsync(
                            progress: new Progress<bool>(_ => tcs.SetResult()));
                        await tcs.Task;
                        await t;
                        test(p.waitForBatch(2));
                    }

                    if (p.ice_getConnection() != null)
                    {
                        test(p.opBatchCount() == 0);
                        Test.TestIntfPrx b1 = (Test.TestIntfPrx)p.ice_batchOneway();
                        b1.opBatch();
                        await b1.ice_getConnection().closeAsync();
                        var tcs = new TaskCompletionSource();
                        Task t = b1.ice_flushBatchRequestsAsync(
                            progress: new Progress<bool>(_ => tcs.SetResult()));
                        await tcs.Task;
                        await t;
                        test(p.waitForBatch(1));
                    }
                }
                output.WriteLine("ok");

                if (p.ice_getConnection() != null)
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
                            var tcs = new TaskCompletionSource();
                            Task t = b1.ice_getConnection().flushBatchRequestsAsync(
                                CompressBatch.BasedOnProxy,
                                progress: new Progress<bool>(_ => tcs.SetResult()));

                            await tcs.Task;
                            await t;
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
                            await b1.ice_getConnection().closeAsync();
                            var tcs = new TaskCompletionSource();
                            Task t = b1.ice_getConnection().flushBatchRequestsAsync(
                                CompressBatch.BasedOnProxy,
                                progress: new Progress<bool>(_ => tcs.SetResult()));
                            try
                            {
                                await t;
                                test(false);
                            }
                            catch (ConnectionClosedException ex)
                            {
                                test(ex.closedByApplication);
                            }
                            test(p.opBatchCount() == 0);
                            test(!tcs.Task.IsCompleted);
                        }
                    }
                    output.WriteLine("ok");
                }

                if (p.ice_getConnection() != null)
                {
                    output.Write("testing batch requests with communicator... ");
                    output.Flush();
                    {
                        {
                            //
                            // Async task - 1 connection.
                            //
                            test(p.opBatchCount() == 0);
                            var b1 = Test.TestIntfPrxHelper.uncheckedCast(
                                p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway());
                            b1.opBatch();
                            b1.opBatch();

                            var tcs = new TaskCompletionSource();
                            Task t = communicator.flushBatchRequestsAsync(
                                CompressBatch.BasedOnProxy,
                                progress: new Progress<bool>(_ => tcs.SetResult()));
                            await tcs.Task;
                            await t;
                            test(t.IsCompleted);
                            test(p.waitForBatch(2));
                        }

                        {
                            //
                            // Async task exception - 1 connection.
                            //
                            test(p.opBatchCount() == 0);
                            var b1 = Test.TestIntfPrxHelper.uncheckedCast(
                                p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway());
                            b1.opBatch();
                            await b1.ice_getConnection().closeAsync();
                            var tcs = new TaskCompletionSource();
                            Task t = communicator.flushBatchRequestsAsync(
                                CompressBatch.BasedOnProxy,
                                progress: new Progress<bool>(_ => tcs.SetResult()));
                            await tcs.Task; // Exceptions are ignored!
                            await t;
                            test(t.IsCompleted);
                            test(p.opBatchCount() == 0);
                        }

                        {
                            //
                            // Async task - 2 connections.
                            //
                            test(p.opBatchCount() == 0);
                            var b1 = Test.TestIntfPrxHelper.uncheckedCast(
                                p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway());
                            var b2 = Test.TestIntfPrxHelper.uncheckedCast(
                                p.ice_connectionId("2").ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway());

                            b2.ice_getConnection(); // Ensure connection is established.
                            b1.opBatch();
                            b1.opBatch();
                            b2.opBatch();
                            b2.opBatch();

                            var tcs = new TaskCompletionSource();
                            Task t = communicator.flushBatchRequestsAsync(
                                CompressBatch.BasedOnProxy,
                                new Progress<bool>(_ => tcs.SetResult()));
                            await tcs.Task;
                            await t;
                            test(t.IsCompleted);
                            test(p.waitForBatch(4));
                        }

                        {
                            //
                            // Async task exception - 2 connections - 1 failure.
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
                            await b1.ice_getConnection().closeAsync();
                            var tcs = new TaskCompletionSource();
                            Task t = communicator.flushBatchRequestsAsync(
                                CompressBatch.BasedOnProxy,
                                new Progress<bool>(_ => tcs.SetResult()));
                            await tcs.Task;
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
                            await b1.ice_getConnection().closeAsync();
                            await b2.ice_getConnection().closeAsync();
                            var tcs = new TaskCompletionSource();
                            Task t = communicator.flushBatchRequestsAsync(
                                CompressBatch.BasedOnProxy,
                                new Progress<bool>(_ => tcs.SetResult()));
                            await tcs.Task;
                            await t;
                            test(t.IsCompleted);
                            test(p.opBatchCount() == 0);
                        }
                    }
                    output.WriteLine("ok");
                }

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
                    catch (Ice.OperationNotExistException)
                    {
                        // Expected with cross testing, this opXxxAsyncDispatch methods are C# only.
                    }
                };
                task().Wait();
                output.WriteLine("ok");

                if (p.ice_getConnection() != null)
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
                            for (int i = 0; i < 200; ++i) // 2MB
                            {
                                cb = new ProgressCallback();
                                _ = p.opWithPayloadAsync(
                                    seq,
                                    progress: cb);
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
                                await t1;
                                test(false);
                            }
                            catch (InvocationCanceledException)
                            {
                            }

                            try
                            {
                                await t2;
                                test(false);
                            }
                            catch (InvocationCanceledException)
                            {
                            }

                            try
                            {
                                await t3;
                                test(false);
                            }
                            catch (InvocationCanceledException)
                            {
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

                if (p.ice_getConnection() != null && p.supportsAMD())
                {
                    output.Write("testing connection close... ");
                    output.Flush();
                    {
                        //
                        // Local case: begin a request, close the connection gracefully, and make sure it waits
                        // for the request to complete.
                        //
                        Connection con = p.ice_getConnection();
                        var tcs = new TaskCompletionSource();
                        con.setCloseCallback(_ => tcs.SetResult());
                        Task t = p.sleepAsync(100);
                        await con.closeAsync();
                        await t; // Should complete successfully.
                        await tcs.Task;
                    }
                    {
                        //
                        // Remote case.
                        //
                        byte[] seq = new byte[1024 * 10];

                        //
                        // Send multiple opWithPayload, followed by a close and followed by multiple opWithPayload.
                        // The goal is to make sure that none of the opWithPayload fail even if the server closes
                        // the connection gracefully in between.
                        //
                        int maxQueue = 2;
                        bool done = false;
                        while (!done && maxQueue < 50)
                        {
                            done = true;
                            p.ice_ping();
                            List<Task> results = new List<Task>();
                            for (int i = 0; i < maxQueue; ++i)
                            {
                                results.Add(p.opWithPayloadAsync(seq));
                            }

                            bool sentSynchronously = true;
                            _ = p.closeConnectionAsync(
                                progress: new Progress<bool>(value => sentSynchronously = value));

                            if (!sentSynchronously)
                            {
                                for (int i = 0; i < maxQueue; i++)
                                {
                                    Task t = p.opWithPayloadAsync(
                                        seq,
                                        progress: new Progress<bool>(value => sentSynchronously = value));
                                    results.Add(t);
                                    if (sentSynchronously)
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
                                await q;
                            }
                            // Wait until the connection is closed.
                            await p.ice_getCachedConnection().closeAsync();
                        }
                    }
                    output.WriteLine("ok");

                    output.Write("testing connection abort... ");
                    output.Flush();
                    {
                        //
                        // Local case: start an operation and then close the connection forcefully on the client side.
                        // There will be no retry and we expect the invocation to fail with ConnectionAbortedException.
                        //
                        p.ice_ping();
                        Connection con = p.ice_getConnection();
                        var tcs = new TaskCompletionSource();
                        Task t = p.startDispatchAsync(
                            progress: new Progress<bool>(_ => tcs.SetResult()));
                        await tcs.Task; // Ensure the request was sent before we close the connection.
                        con.abort();
                        try
                        {
                            await t;
                            test(false);
                        }
                        catch (ConnectionAbortedException ex)
                        {
                            test(ex.closedByApplication);
                        }
                        p.finishDispatch();

                        //
                        // Remote case: the server closes the connection forcefully. This causes the request to fail
                        // with a ConnectionLostException. Since the close() operation is not idempotent, the client
                        // will not retry.
                        //
                        try
                        {
                            p.abortConnection();
                            test(false);
                        }
                        catch (Ice.ConnectionLostException)
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
                }
                output.WriteLine("ok");

                if (!collocated)
                {
                    output.Write("testing bi-dir... ");
                    ObjectAdapter adapter = communicator.createObjectAdapter("");
                    var replyI = new PingReplyI();
                    var reply = Test.PingReplyPrxHelper.uncheckedCast(adapter.addWithUUID(replyI));

                    var context = new Dictionary<string, string> { ["ONE"] = "" };
                    await p.pingBiDirAsync(reply, context);

                    p.ice_getConnection().setAdapter(adapter);
                    await p.pingBiDirAsync(reply);
                    test(replyI.checkReceived());
                    adapter.destroy();
                    output.WriteLine("ok");
                }

                output.Write("testing result struct... ");
                output.Flush();
                {
                    var q = Test.Outer.Inner.TestIntfPrxHelper.createProxy(
                        communicator, "test2:" + helper.getTestEndpoint(0));
                    var r = await q.opAsync(1);
                    test(r.returnValue == 1);
                    test(r.j == 1);
                }
                output.WriteLine("ok");

                if (p.supportsBackPressureTests())
                {
                    output.Write("testing back pressure... ");
                    output.Flush();
                    {
                        // Keep the 3 server thread pool threads busy.
                        Task sleep1Task = p.sleepAsync(1000);
                        Task sleep2Task = p.sleepAsync(1000);
                        Task sleep3Task = p.sleepAsync(1000);
                        bool canceled = false;
                        using var cts = new CancellationTokenSource(200);
                        try
                        {
                            var onewayProxy = (Test.TestIntfPrx)p.ice_oneway();

                            // Sending should be canceled because the TCP send/receive buffer size on the server is set
                            // to 50KB. Note: we don't use the cancel parameter of the operation here because the
                            // cancellation doesn't cancel the operation whose payload is being sent.
                            await onewayProxy.opWithPayloadAsync(new byte[768 * 1024]).WaitAsync(cts.Token);
                        }
                        catch (OperationCanceledException)
                        {
                            canceled = true;
                        }
                        test(canceled && !sleep1Task.IsCompleted);
                        await sleep1Task;
                        await sleep2Task;
                        await sleep3Task;
                    }
                    output.WriteLine("ok");
                }

                p.shutdown();
            }
        }
    }
}
