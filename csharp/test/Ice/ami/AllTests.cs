// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.AMI
{
    public static class AllTests
    {
        public class Progress : IProgress<bool>
        {
            public Progress(Action<bool> report) => _report = report;

            public void Report(bool sentSynchronously) => _report(sentSynchronously);

            private readonly Action<bool> _report;
        }

        public class ProgressCallback : IProgress<bool>
        {
            private readonly object _mutex = new();
            private bool _sent;
            private bool _sentSynchronously;

            public bool Sent
            {
                get
                {
                    lock (_mutex)
                    {
                        return _sent;
                    }
                }
                set
                {
                    lock (_mutex)
                    {
                        _sent = value;
                    }
                }
            }

            public bool SentSynchronously
            {
                get
                {
                    lock (_mutex)
                    {
                        return _sentSynchronously;
                    }
                }
                set
                {
                    lock (_mutex)
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
        }

        private class CallbackBase
        {
            private bool _called;
            private readonly object _mutex = new();

            public virtual void Check()
            {
                lock (_mutex)
                {
                    while (!_called)
                    {
                        Monitor.Wait(_mutex);
                    }
                    _called = false;
                }
            }

            public virtual void Called()
            {
                lock (_mutex)
                {
                    TestHelper.Assert(!_called);
                    _called = true;
                    Monitor.Pulse(_mutex);
                }
            }
        }

        public static async Task RunAsync(TestHelper helper)
        {
            Communicator communicator = helper.Communicator;

            bool ice1 = helper.Protocol == Protocol.Ice1;

            var p = ITestIntfPrx.Parse(helper.GetTestProxy("test", 0), communicator);
            var serialized = ITestIntfPrx.Parse(helper.GetTestProxy("serialized", 1), communicator);

            TextWriter output = helper.Output;

            output.Write("testing async invocation...");
            output.Flush();
            {
                var ctx = new Dictionary<string, string>();

                TestHelper.Assert(p.IceIsAAsync("::ZeroC::Ice::Test::AMI::TestIntf").Result);
                TestHelper.Assert(p.IceIsAAsync("::ZeroC::Ice::Test::AMI::TestIntf", ctx).Result);

                p.IcePingAsync().Wait();
                p.IcePingAsync(ctx).Wait();

                TestHelper.Assert(p.IceIdAsync().Result.Equals("::ZeroC::Ice::Test::AMI::TestIntf"));
                TestHelper.Assert(p.IceIdAsync(ctx).Result.Equals("::ZeroC::Ice::Test::AMI::TestIntf"));

                TestHelper.Assert(p.IceIdsAsync().Result.Length == 2);
                TestHelper.Assert(p.IceIdsAsync(ctx).Result.Length == 2);

                p.OpAsync().Wait();
                p.OpAsync(ctx).Wait();

                TestHelper.Assert(p.OpWithResultAsync().Result == 15);
                TestHelper.Assert(p.OpWithResultAsync(ctx).Result == 15);

                try
                {
                    p.OpWithUEAsync().Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ae)
                {
                    ae.Handle(ex => ex is TestIntfException);
                }

                try
                {
                    p.OpWithUEAsync(ctx).Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ae)
                {
                    ae.Handle(ex => ex is TestIntfException);
                }
            }
            output.WriteLine("ok");

            output.Write("testing async/await...");
            output.Flush();
            {
                Task.Run(async () =>
                    {
                        var ctx = new Dictionary<string, string>();

                        TestHelper.Assert(await p.IceIsAAsync("::ZeroC::Ice::Test::AMI::TestIntf"));
                        TestHelper.Assert(await p.IceIsAAsync("::ZeroC::Ice::Test::AMI::TestIntf", ctx));

                        await p.IcePingAsync();
                        await p.IcePingAsync(ctx);

                        string id = await p.IceIdAsync();
                        TestHelper.Assert(id.Equals("::ZeroC::Ice::Test::AMI::TestIntf"));
                        id = await p.IceIdAsync(ctx);
                        TestHelper.Assert(id.Equals("::ZeroC::Ice::Test::AMI::TestIntf"));

                        string[] ids = await p.IceIdsAsync();
                        TestHelper.Assert(ids.Length == 2);
                        ids = await p.IceIdsAsync(ctx);
                        TestHelper.Assert(ids.Length == 2);

                        await p.OpAsync();
                        await p.OpAsync(ctx);

                        int result = await p.OpWithResultAsync();
                        TestHelper.Assert(result == 15);
                        result = await p.OpWithResultAsync(ctx);
                        TestHelper.Assert(result == 15);

                        try
                        {
                            await p.OpWithUEAsync();
                            TestHelper.Assert(false);
                        }
                        catch (Exception ex)
                        {
                            TestHelper.Assert(ex is TestIntfException);
                        }

                        try
                        {
                            await p.OpWithUEAsync(ctx);
                            TestHelper.Assert(false);
                        }
                        catch (Exception ex)
                        {
                            TestHelper.Assert(ex is TestIntfException);
                        }
                    }).Wait();
            }
            output.WriteLine("ok");

            output.Write("testing async continuations...");
            output.Flush();
            {
                var ctx = new Dictionary<string, string>();

                p.IceIsAAsync("::ZeroC::Ice::Test::AMI::TestIntf").ContinueWith(
                    previous => TestHelper.Assert(previous.Result), TaskScheduler.Default).Wait();

                p.IceIsAAsync("::ZeroC::Ice::Test::AMI::TestIntf", ctx).ContinueWith(
                    previous => TestHelper.Assert(previous.Result), TaskScheduler.Default).Wait();

                p.IcePingAsync().ContinueWith(previous => previous.Wait(), TaskScheduler.Default).Wait();

                p.IcePingAsync(ctx).ContinueWith(previous => previous.Wait(), TaskScheduler.Default).Wait();

                p.IceIdAsync().ContinueWith(
                    previous => TestHelper.Assert(previous.Result == "::ZeroC::Ice::Test::AMI::TestIntf"),
                    TaskScheduler.Default).Wait();

                p.IceIdAsync(ctx).ContinueWith(
                    previous => TestHelper.Assert(previous.Result == "::ZeroC::Ice::Test::AMI::TestIntf"),
                    TaskScheduler.Default).Wait();

                p.IceIdsAsync().ContinueWith(previous => TestHelper.Assert(previous.Result.Length == 2),
                                             TaskScheduler.Default).Wait();

                p.IceIdsAsync(ctx).ContinueWith(previous => TestHelper.Assert(previous.Result.Length == 2),
                                                TaskScheduler.Default).Wait();

                p.OpAsync().ContinueWith(previous => previous.Wait(), TaskScheduler.Default).Wait();
                p.OpAsync(ctx).ContinueWith(previous => previous.Wait(), TaskScheduler.Default).Wait();

                p.OpWithResultAsync().ContinueWith(
                    previous => TestHelper.Assert(previous.Result == 15), TaskScheduler.Default).Wait();

                p.OpWithResultAsync(ctx).ContinueWith(previous => TestHelper.Assert(previous.Result == 15),
                                                      TaskScheduler.Default).Wait();

                p.OpWithUEAsync().ContinueWith(
                    previous =>
                    {
                        try
                        {
                            previous.Wait();
                        }
                        catch (AggregateException ae)
                        {
                            ae.Handle(ex => ex is TestIntfException);
                        }
                    },
                    TaskScheduler.Default).Wait();

                p.OpWithUEAsync(ctx).ContinueWith(
                    previous =>
                    {
                        try
                        {
                            previous.Wait();
                        }
                        catch (AggregateException ae)
                        {
                            ae.Handle(ex => ex is TestIntfException);
                        }
                    },
                    TaskScheduler.Default).Wait();
            }
            output.WriteLine("ok");

            output.Write("testing local exceptions with async tasks... ");
            output.Flush();
            {
                ITestIntfPrx indirect = ice1 ?
                    p.Clone(location: ImmutableArray.Create("dummy")) :
                    p.Clone(endpoints: ImmutableArray<Endpoint>.Empty, location: ImmutableArray.Create("dummy"));

                try
                {
                    indirect.OpAsync().Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ex)
                {
                    TestHelper.Assert(ex.InnerException is NoEndpointException);
                }

                Communicator ic = TestHelper.CreateCommunicator(communicator.GetProperties());
                var p2 = ITestIntfPrx.Parse(p.ToString()!, ic);
                await ic.DisposeAsync();

                try
                {
                    p2.OpAsync().Wait();
                    TestHelper.Assert(false);
                }
                catch (CommunicatorDisposedException)
                {
                }
            }
            output.WriteLine("ok");

            output.Write("testing exception with async task... ");
            output.Flush();
            {
                ITestIntfPrx i = ice1 ?
                    p.Clone(location: ImmutableArray.Create("dummy")) :
                    p.Clone(endpoints: ImmutableArray<Endpoint>.Empty, location: ImmutableArray.Create("dummy"));

                try
                {
                    i.IceIsAAsync("::ZeroC::Ice::Test::AMI::TestIntf").Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ex)
                {
                    TestHelper.Assert(ex.InnerException is NoEndpointException);
                }

                try
                {
                    i.OpAsync().Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ex)
                {
                    TestHelper.Assert(ex.InnerException is NoEndpointException);
                }

                try
                {
                    i.OpWithResultAsync().Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ex)
                {
                    TestHelper.Assert(ex.InnerException is NoEndpointException);
                }

                try
                {
                    i.OpWithUEAsync().Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ex)
                {
                    TestHelper.Assert(ex.InnerException is NoEndpointException);
                }

                // Ensures no exception is called when response is received
                TestHelper.Assert(p.IceIsAAsync("::ZeroC::Ice::Test::AMI::TestIntf").Result);
                p.OpAsync().Wait();
                p.OpWithResultAsync().Wait();

                // If response is a user exception, it should be received.
                try
                {
                    p.OpWithUEAsync().Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ae)
                {
                    ae.Handle(ex => ex is TestIntfException);
                }
            }
            output.WriteLine("ok");

            output.Write("testing progress callback... ");
            output.Flush();
            {
                {
                    var cb = new CallbackBase();

                    Task t = p.IceIsAAsync("",
                        progress: new Progress(sentSynchronously => cb.Called()));
                    cb.Check();
                    t.Wait();

                    t = p.IcePingAsync(progress: new Progress(sentSynchronously => cb.Called()));
                    cb.Check();
                    t.Wait();

                    t = p.IceIdAsync(progress: new Progress(sentSynchronously => cb.Called()));
                    cb.Check();
                    t.Wait();

                    t = p.IceIdsAsync(progress: new Progress(sentSynchronously => cb.Called()));
                    cb.Check();
                    t.Wait();

                    t = p.OpAsync(progress: new Progress(sentSynchronously => cb.Called()));
                    cb.Check();
                    t.Wait();
                }

                var tasks = new List<Task>();
                byte[] seq = new byte[1000 * 1024];
                new Random().NextBytes(seq);
                {
                    Task t;
                    ProgressCallback cb;
                    do
                    {
                        cb = new ProgressCallback();
                        t = p.OpWithPayloadAsync(seq, progress: cb);
                        tasks.Add(t);
                    }
                    while (cb.SentSynchronously);
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
                    await p.OpAsync();

                    // Run blocking IcePing() on another thread from the continuation to ensure there's no deadlock
                    // if the continuaion blocks and wait for another thread to complete an invocation with the
                    // connection.
                    Task.Run(() => p.IcePing()).Wait();

                    int r = await p.OpWithResultAsync();
                    TestHelper.Assert(r == 15);

                    try
                    {
                        await p.OpWithUEAsync();
                        TestHelper.Assert(false);
                    }
                    catch (TestIntfException)
                    {
                        // Run blocking IcePing() on another thread from the continuation to ensure there's no deadlock
                        // if the continuation blocks and wait for another thread to complete an invocation with the
                        // connection.
                        Task.Run(() => p.IcePing()).Wait();
                    }

                    try
                    {
                        await p.CloseAsync(CloseMode.Forcefully);
                        TestHelper.Assert(false);
                    }
                    catch
                    {
                        // Run blocking IcePing() on another thread from the continuation to ensure there's no deadlock
                        // if the continuation blocks and wait for another thread to complete an invocation with the
                        // connection.
                        Task.Run(() => p.IcePing()).Wait();
                    }

                    // Operations implemented with amd and async.
                    await p.OpAsyncDispatchAsync();

                    r = await p.OpWithResultAsyncDispatchAsync();
                    TestHelper.Assert(r == 15);

                    try
                    {
                        await p.OpWithUEAsyncDispatchAsync();
                        TestHelper.Assert(false);
                    }
                    catch (TestIntfException)
                    {
                    }

                    await p.OpAsync();

                    // Run blocking IcePing() on another thread from the continuation to ensure there's no deadlock
                    // if the continuaion blocks and wait for another thread to complete an invocation with the
                    // connection.
                    Task.Run(() => p.IcePing()).Wait();
                }
                catch (OperationNotExistException)
                {
                    // Expected with cross testing, this opXxxAsyncDispatch methods are C# only.
                }
            };
            task().Wait();
            output.WriteLine("ok");

            Task.Run(async () =>
            {
                output.Write("testing async serialization... ");
                output.Flush();
                try
                {
                    {
                        int previous = 0;
                        int expected = 0;
                        var tasks = new Task<int>[20];
                        var context = new Dictionary<string, string>();
                        for (int i = 0; i < 50; ++i)
                        {
                            // Async serialization only works once the connection is established and if there's no retries
                            serialized.IcePing();
                            for (int j = 0; j < tasks.Length; ++j)
                            {
                                context["value"] = j.ToString(); // This is for debugging
                                tasks[j] = serialized.SetAsync(j, context);
                            }
                            for (int j = 0; j < tasks.Length; ++j)
                            {
                                previous = await tasks[j].ConfigureAwait(false);
                                TestHelper.Assert(previous == expected);
                                expected = j;
                            }
                            await (await serialized.GetConnectionAsync()).GoAwayAsync();
                        }
                    }

                    {
                        // Ensure requests don't fail if the connection is closed gracefully while being sent. The
                        // ping calls will get retried.
                        var tasks = new Task[20];
                        for (int i = 0; i < 10; ++i)
                        {
                            serialized.IcePing();
                            for (int j = 0; j < tasks.Length; ++j)
                            {
                                tasks[j] = serialized.IcePingAsync();
                            }
                            _ = (await serialized.GetConnectionAsync()).GoAwayAsync();
                            for (int j = 0; j < tasks.Length; ++j)
                            {
                                await tasks[j].ConfigureAwait(false);
                            }
                        }
                    }
                }
                catch (ObjectNotExistException)
                {
                    output.WriteLine("not supported");
                }
                catch (Exception ex)
                {
                    output.WriteLine($"unexpected exception {ex}");
                    TestHelper.Assert(false);
                }

                try
                {
                    {
                        var tasks = new Task[20];
                        serialized.Set(-1);
                        var context = new Dictionary<string, string>();
                        for (int i = 0; i < 50; ++i)
                        {
                            // Async serialization only works once the connection is established and if there's no retries
                            serialized.IcePing();
                            for (int j = 0; j < tasks.Length; ++j)
                            {
                                context["value"] = (i * tasks.Length + j).ToString(); // This is for debugging
                                tasks[j] = serialized.SetOnewayAsync(i * tasks.Length + j - 1,
                                                                    i * tasks.Length + j,
                                                                    context);
                            }
                            for (int j = 0; j < tasks.Length; ++j)
                            {
                                await tasks[j].ConfigureAwait(false);
                            }
                            await (await serialized.GetConnectionAsync()).GoAwayAsync();
                        }
                    }

                    {
                        // Ensure requests don't fail if the connection is closed gracefully while being sent. The
                        // ping calls will get retried.
                        var tasks = new Task[20];
                        var serializedOneway = serialized.Clone(oneway: true);
                        for (int i = 0; i < 10; ++i)
                        {
                            serializedOneway.IcePing();
                            for (int j = 0; j < tasks.Length; ++j)
                            {
                                tasks[j] = serializedOneway.IcePingAsync();
                            }
                            _ = (await serializedOneway.GetConnectionAsync()).GoAwayAsync();
                            for (int j = 0; j < tasks.Length; ++j)
                            {
                                await tasks[j].ConfigureAwait(false);
                            }
                        }
                    }
                }
                catch (ObjectNotExistException)
                {
                    output.WriteLine("not supported");
                }
                catch (Exception ex)
                {
                    output.WriteLine($"unexpected exception {ex}");
                    TestHelper.Assert(false);
                }
                output.WriteLine("ok");
            }).Wait();

            output.Write("testing async Task cancellation... ");
            output.Flush();
            {
                var cs1 = new CancellationTokenSource();
                var cs2 = new CancellationTokenSource();
                var cs3 = new CancellationTokenSource();
                Task t1;
                Task t2;
                Task? t3;
                try
                {
                    var cancelCtx = new Dictionary<string, string> { { "cancel", "" } };
                    t1 = p.SleepAsync(1000, cancel: cs1.Token, context: cancelCtx);
                    t2 = p.SleepAsync(1000, cancel: cs2.Token, context: cancelCtx);
                    cs1.Cancel();
                    cs2.Cancel();
                    cs3.Cancel();
                    try
                    {
                        t3 = p.IcePingAsync(cancel: cs3.Token);
                        // It might throw synchronously or asynchronously depending on connection establishment
                    }
                    catch (OperationCanceledException)
                    {
                        // expected
                        t3 = null;
                    }
                    try
                    {
                        t1.Wait();
                        TestHelper.Assert(false);
                    }
                    catch (AggregateException ae)
                    {
                        ae.Handle(ex => ex is OperationCanceledException);
                    }
                    try
                    {
                        t2.Wait();
                        TestHelper.Assert(false);
                    }
                    catch (AggregateException ae)
                    {
                        ae.Handle(ex => ex is OperationCanceledException);
                    }
                    if (t3 != null)
                    {
                        try
                        {
                            t3.Wait();
                            TestHelper.Assert(false);
                        }
                        catch (AggregateException ae)
                        {
                            ae.Handle(ex => ex is OperationCanceledException);
                        }
                    }
                }
                finally
                {
                    p.IcePing();
                }
            }
            {
                // Stress test cancellation to ensure we exercise the various cancellation points. Cancellation of
                // the sleep might fail or succeed on the server side depending how long we sleep.
                var cancelCtx = new Dictionary<string, string> { { "cancel", "mightSucceed" } };
                for (int i = 0; i < 20; ++i)
                {
                    var source = new CancellationTokenSource();
                    source.CancelAfter(TimeSpan.FromMilliseconds(i));
                    try
                    {
                        p.Clone().SleepAsync(2000, cancel: source.Token, context: cancelCtx).Wait();
                        TestHelper.Assert(false);
                    }
                    catch (OperationCanceledException)
                    {
                        // expected
                    }
                    catch (AggregateException ae)
                    {
                        ae.Handle(ex => ex is OperationCanceledException);
                    }
                }

                // Set the value on the servant to 20 and sleep for 500ms. We send a large payload to fill up the
                // send buffer and ensure other requests won't be sent.
                serialized.Set(20);
                _ = serialized.SleepAsync(400);
                var payload = new byte[5 * 1024 * 1024];
                _ = serialized.OpWithPayloadAsync(payload);
                _ = serialized.OpWithPayloadAsync(payload);
                _ = serialized.OpWithPayloadAsync(payload);
                _ = serialized.OpWithPayloadAsync(payload);
                _ = serialized.OpWithPayloadAsync(payload);
                _ = serialized.OpWithPayloadAsync(payload);

                // The send queue is blocked, we send 4 set requests and cancel 2 of them. We make sure that the
                // requests are canceled and not sent by checking the response of set which sends the previous set
                // value.
                var source0 = new CancellationTokenSource();
                Task<int> t0 = serialized.SetAsync(0, cancel: source0.Token);
                Task<int> t1 = serialized.SetAsync(1);
                var source2 = new CancellationTokenSource();
                Task<int> t2 = serialized.SetAsync(2, cancel: source2.Token);
                Task<int> t3 = serialized.SetAsync(3);
                source0.Cancel();
                source2.Cancel();
                try
                {
                    t0.Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ex) when (ex.InnerException is OperationCanceledException)
                {
                }
                try
                {
                    t2.Wait();
                }
                catch (AggregateException ex) when (ex.InnerException is OperationCanceledException)
                {
                }
                TestHelper.Assert(t0.Status == TaskStatus.Canceled);
                TestHelper.Assert(t2.Status == TaskStatus.Canceled);
                TestHelper.Assert(t1.Result == 20);
                TestHelper.Assert(t3.Result == 1);
                TestHelper.Assert(serialized.Set(0) == 3);
            }
            output.WriteLine("ok");

            if (p.SupportsAMD())
            {
                output.Write("testing graceful close connection... ");
                output.Flush();
                {
                    // Remote case.
                    byte[] seq = new byte[1024 * 10];

                    // Send multiple opWithPayload, followed by a close and followed by multiple opWithPaylod. The goal
                    // is to make sure that none of the opWithPayload fail even if the server closes the connection
                    // gracefully in between.
                    int maxQueue = 2;
                    bool done = false;
                    while (!done && maxQueue < 50)
                    {
                        done = true;
                        p.IcePing();
                        var results = new List<Task>();
                        for (int i = 0; i < maxQueue; ++i)
                        {
                            results.Add(p.OpWithPayloadAsync(seq));
                        }

                        var cb = new ProgressCallback();
                        _ = p.CloseAsync(CloseMode.Gracefully, progress: cb);

                        if (!cb.Sent)
                        {
                            for (int i = 0; i < maxQueue; i++)
                            {
                                cb = new ProgressCallback();
                                Task t = p.OpWithPayloadAsync(seq, progress: cb);
                                results.Add(t);
                                if (cb.Sent)
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

                {
                    // Local case: start an operation and then close the connection gracefully on the client side
                    // without waiting for the pending invocation to complete. There will be no retry and we expect the
                    // invocation to fail with ConnectionClosedException.
                    p = p.Clone(label: "CloseGracefully"); // Start with a new connection.
                    Connection con = await p.GetConnectionAsync();
                    var cb = new CallbackBase();
                    Task t = p.StartDispatchAsync(progress: new Progress(sentSynchronously => cb.Called()));
                    cb.Check(); // Ensure the request was sent before we close the connection.
                    _ = con.GoAwayAsync();
                    try
                    {
                        t.Wait();
                        TestHelper.Assert(false);
                    }
                    catch (AggregateException ex)
                    {
                        TestHelper.Assert(ex.InnerException is ConnectionClosedException);
                    }
                    p.FinishDispatch();

                    // Remote case: the server closes the connection gracefully, which means the connection will not
                    // be closed until all pending dispatched requests have completed.
                    con = await p.GetConnectionAsync();
                    cb = new CallbackBase();
                    con.Closed += (sender, args) => cb.Called();
                    t = p.SleepAsync(100);
                    _ = p.CloseAsync(CloseMode.Gracefully); // Close is delayed until sleep completes.
                    cb.Check();
                    t.Wait();
                }
                output.WriteLine("ok");

                output.Write("testing forceful close connection... ");
                output.Flush();
                {
                    // Local case: start an operation and then close the connection forcefully on the client side.
                    // There will be no retry and we expect the invocation to fail with ConnectionClosedLocallyException.
                    await p.IcePingAsync();
                    Connection con = await p.GetConnectionAsync();
                    var cb = new CallbackBase();
                    Task t = p.StartDispatchAsync(
                        progress: new Progress(sentSynchronously => cb.Called()));
                    cb.Check(); // Ensure the request was sent before we close the connection.
                    _ = con.AbortAsync();
                    try
                    {
                        t.Wait();
                        TestHelper.Assert(false);
                    }
                    catch (AggregateException ex)
                    {
                        TestHelper.Assert(ex.InnerException is ConnectionClosedException);
                    }
                    p.FinishDispatch();

                    // Remote case: the server closes the connection forcefully. This causes the request to fail with
                    // a ConnectionLostException. Since the close() operation is not idempotent, the client will not
                    // retry.
                    try
                    {
                        p.Close(CloseMode.Forcefully);
                        TestHelper.Assert(false);
                    }
                    catch (ConnectionLostException)
                    {
                        // Expected.
                    }
                }
                output.WriteLine("ok");
            }

            output.Write("testing result struct... ");
            output.Flush();
            {
                var q = Outer.Inner.ITestIntfPrx.Parse(helper.GetTestProxy("test2", 0), communicator);
                q.OpAsync(1).ContinueWith(t =>
                    {
                        (int ReturnValue, int j) = t.Result;
                        TestHelper.Assert(ReturnValue == 1);
                        TestHelper.Assert(j == 1);
                    },
                    TaskScheduler.Default).Wait();
            }
            output.WriteLine("ok");

            await p.ShutdownAsync();
        }
    }
}
