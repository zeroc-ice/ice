//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.AMI
{
    public class AllTests
    {
        public class Progress : IProgress<bool>
        {
            public Progress(Action<bool> report) => _report = report;

            public void Report(bool sentSynchronously) => _report(sentSynchronously);

            private readonly Action<bool> _report;
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

        private class CallbackBase
        {
            internal CallbackBase() => _called = false;

            public virtual void Check()
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

            public virtual void Called()
            {
                lock (this)
                {
                    TestHelper.Assert(!_called);
                    _called = true;
                    Monitor.Pulse(this);
                }
            }

            private bool _called;
        }

        private class SentCallback : CallbackBase
        {
            public SentCallback() => _thread = Thread.CurrentThread;

            public void Sent(bool ss)
            {
                TestHelper.Assert((ss && _thread == Thread.CurrentThread) || (!ss && _thread != Thread.CurrentThread));

                Called();
            }

            private readonly Thread _thread;
        }

        public static void allTests(TestHelper helper, bool collocated)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);

            var p = ITestIntfPrx.Parse($"test:{helper.GetTestEndpoint(0)}", communicator);
            var serialized = ITestIntfPrx.Parse($"serialized:{helper.GetTestEndpoint(1)}", communicator);

            TextWriter output = helper.GetWriter();

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

                if (!collocated)
                {
                    TestHelper.Assert(p.GetConnectionAsync().Result != null);
                }

                p.opAsync().Wait();
                p.opAsync(ctx).Wait();

                TestHelper.Assert(p.opWithResultAsync().Result == 15);
                TestHelper.Assert(p.opWithResultAsync(ctx).Result == 15);

                try
                {
                    p.opWithUEAsync().Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ae)
                {
                    ae.Handle(ex => ex is TestIntfException);
                }

                try
                {
                    p.opWithUEAsync(ctx).Wait();
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

                        if (!collocated)
                        {
                            Connection? conn = await p.GetConnectionAsync();
                            TestHelper.Assert(conn != null);
                        }

                        await p.opAsync();
                        await p.opAsync(ctx);

                        int result = await p.opWithResultAsync();
                        TestHelper.Assert(result == 15);
                        result = await p.opWithResultAsync(ctx);
                        TestHelper.Assert(result == 15);

                        try
                        {
                            await p.opWithUEAsync();
                            TestHelper.Assert(false);
                        }
                        catch (Exception ex)
                        {
                            TestHelper.Assert(ex is TestIntfException);
                        }

                        try
                        {
                            await p.opWithUEAsync(ctx);
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

                p.IceIsAAsync("::ZeroC::Ice::Test::AMI::TestIntf").ContinueWith(previous => TestHelper.Assert(previous.Result)).Wait();

                p.IceIsAAsync("::ZeroC::Ice::Test::AMI::TestIntf", ctx).ContinueWith(previous => TestHelper.Assert(previous.Result)).Wait();

                p.IcePingAsync().ContinueWith(previous => previous.Wait()).Wait();

                p.IcePingAsync(ctx).ContinueWith(previous => previous.Wait()).Wait();

                p.IceIdAsync().ContinueWith(previous => TestHelper.Assert(previous.Result.Equals("::ZeroC::Ice::Test::AMI::TestIntf"))).Wait();

                p.IceIdAsync(ctx).ContinueWith(previous => TestHelper.Assert(previous.Result.Equals("::ZeroC::Ice::Test::AMI::TestIntf"))).Wait();

                p.IceIdsAsync().ContinueWith(previous => TestHelper.Assert(previous.Result.Length == 2)).Wait();

                p.IceIdsAsync(ctx).ContinueWith(previous => TestHelper.Assert(previous.Result.Length == 2)).Wait();

                if (!collocated)
                {
                    p.GetConnectionAsync().AsTask().ContinueWith(previous => TestHelper.Assert(previous.Result != null)).Wait();
                }

                p.opAsync().ContinueWith(previous => previous.Wait()).Wait();
                p.opAsync(ctx).ContinueWith(previous => previous.Wait()).Wait();

                p.opWithResultAsync().ContinueWith(previous => TestHelper.Assert(previous.Result == 15)).Wait();

                p.opWithResultAsync(ctx).ContinueWith(previous => TestHelper.Assert(previous.Result == 15)).Wait();

                p.opWithUEAsync().ContinueWith(previous =>
                    {
                        try
                        {
                            previous.Wait();
                        }
                        catch (AggregateException ae)
                        {
                            ae.Handle(ex => ex is TestIntfException);
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
                            ae.Handle(ex => ex is TestIntfException);
                        }
                    }).Wait();
            }
            output.WriteLine("ok");

            output.Write("testing local exceptions with async tasks... ");
            output.Flush();
            {
                ITestIntfPrx indirect = p.Clone(adapterId: "dummy");

                try
                {
                    indirect.opAsync().Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ex)
                {
                    TestHelper.Assert(ex.InnerException is NoEndpointException);
                }

                //
                // Check that CommunicatorDestroyedException is raised directly.
                //
                if (p.GetConnection() != null)
                {
                    Communicator ic = helper.Initialize(communicator.GetProperties());
                    var p2 = ITestIntfPrx.Parse(p.ToString()!, ic);
                    TestHelper.Assert(p2 != null);
                    ic.Destroy();

                    try
                    {
                        p2.opAsync();
                        TestHelper.Assert(false);
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
                ITestIntfPrx i = p.Clone(adapterId: "dummy");

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
                    i.opAsync().Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ex)
                {
                    TestHelper.Assert(ex.InnerException is NoEndpointException);
                }

                try
                {
                    i.opWithResultAsync().Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ex)
                {
                    TestHelper.Assert(ex.InnerException is NoEndpointException);
                }

                try
                {
                    i.opWithUEAsync().Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ex)
                {
                    TestHelper.Assert(ex.InnerException is NoEndpointException);
                }

                // Ensures no exception is called when response is received
                TestHelper.Assert(p.IceIsAAsync("::ZeroC::Ice::Test::AMI::TestIntf").Result);
                p.opAsync().Wait();
                p.opWithResultAsync().Wait();

                // If response is a user exception, it should be received.
                try
                {
                    p.opWithUEAsync().Wait();
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
                    var cb = new SentCallback();

                    Task t = p.IceIsAAsync("",
                        progress: new Progress(sentSynchronously => cb.Sent(sentSynchronously)));
                    cb.Check();
                    t.Wait();

                    t = p.IcePingAsync(progress: new Progress(sentSynchronously => cb.Sent(sentSynchronously)));
                    cb.Check();
                    t.Wait();

                    t = p.IceIdAsync(progress: new Progress(sentSynchronously => cb.Sent(sentSynchronously)));
                    cb.Check();
                    t.Wait();

                    t = p.IceIdsAsync(progress: new Progress(sentSynchronously => cb.Sent(sentSynchronously)));
                    cb.Check();
                    t.Wait();

                    t = p.opAsync(progress: new Progress(sentSynchronously => cb.Sent(sentSynchronously)));
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
                        t = p.opWithPayloadAsync(seq, progress: cb);
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
                    await p.opAsync();

                    int r = await p.opWithResultAsync();
                    TestHelper.Assert(r == 15);

                    try
                    {
                        await p.opWithUEAsync();
                    }
                    catch (TestIntfException)
                    {
                    }

                    // Operations implemented with amd and async.
                    await p.opAsyncDispatchAsync();

                    r = await p.opWithResultAsyncDispatchAsync();
                    TestHelper.Assert(r == 15);

                    try
                    {
                        await p.opWithUEAsyncDispatchAsync();
                        TestHelper.Assert(false);
                    }
                    catch (TestIntfException)
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

            Task.Run(async () =>
            {
                if (serialized.GetConnection() == null)
                {
                    return; // Serialization not supported with collocation
                }

                output.Write("testing async serialization... ");
                output.Flush();
                try
                {
                    int previous = 0;
                    int expected = 0;
                    var tasks = new Task<int>[20];
                    var context = new Dictionary<string, string>();
                    for (int i = 0; i < 50; ++i)
                    {
                        // Async serialization only works once the connection is established and if there's no
                        // retries
                        serialized.IcePing();
                        for (int j = 0; j < tasks.Length; ++j)
                        {
                            context["value"] = j.ToString(); // This is for debugging
                            tasks[j] = serialized.setAsync(j, context);
                        }
                        for (int j = 0; j < tasks.Length; ++j)
                        {
                            previous = await tasks[j].ConfigureAwait(false);
                            TestHelper.Assert(previous == expected);
                            expected = j;
                        }
                        serialized.GetConnection()!.Close(ConnectionClose.Gracefully);
                    }
                    output.WriteLine("ok");
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
            }).Wait();

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
                        t1 = p.sleepAsync(300, cancel: cs1.Token);
                        t2 = p.sleepAsync(300, cancel: cs2.Token);
                        cs1.Cancel();
                        cs2.Cancel();
                        cs3.Cancel();
                        try
                        {
                            t3 = p.IcePingAsync(cancel: cs3.Token);
                        }
                        catch (OperationCanceledException)
                        {
                            // expected
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
                    }
                    finally
                    {
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
                    Connection con = p.GetConnection()!;
                    var cb = new CallbackBase();
                    con.SetCloseCallback(_ => cb.Called());
                    Task t = p.sleepAsync(100);
                    con.Close(ConnectionClose.GracefullyWithWait);
                    t.Wait(); // Should complete successfully.
                    cb.Check();
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
                        var results = new List<Task>();
                        for (int i = 0; i < maxQueue; ++i)
                        {
                            results.Add(p.opWithPayloadAsync(seq));
                        }

                        var cb = new ProgressCallback();
                        p.closeAsync(CloseMode.GracefullyWithWait, progress: cb);

                        if (!cb.SentSynchronously)
                        {
                            for (int i = 0; i < maxQueue; i++)
                            {
                                cb = new ProgressCallback();
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
                    // invocation to fail with ConnectionClosedLocallyException.
                    //
                    p = p.Clone(connectionId: "CloseGracefully"); // Start with a new connection.
                    Connection con = p.GetConnection()!;
                    var cb = new CallbackBase();
                    Task t = p.startDispatchAsync(
                        progress: new Progress(sentSynchronously => cb.Called()));
                    cb.Check(); // Ensure the request was sent before we close the connection.
                    con.Close(ConnectionClose.Gracefully);
                    try
                    {
                        t.Wait();
                        TestHelper.Assert(false);
                    }
                    catch (AggregateException ex)
                    {
                        TestHelper.Assert(ex.InnerException is ConnectionClosedLocallyException);
                    }
                    p.finishDispatch();

                    //
                    // Remote case: the server closes the connection gracefully, which means the connection
                    // will not be closed until all pending dispatched requests have completed.
                    //
                    con = p.GetConnection()!;
                    cb = new CallbackBase();
                    con.SetCloseCallback(_ => cb.Called());
                    t = p.sleepAsync(100);
                    p.close(CloseMode.Gracefully); // Close is delayed until sleep completes.
                    cb.Check();
                    t.Wait();
                }
                output.WriteLine("ok");

                output.Write("testing forceful close connection... ");
                output.Flush();
                {
                    //
                    // Local case: start an operation and then close the connection forcefully on the client side.
                    // There will be no retry and we expect the invocation to fail with ConnectionClosedLocallyException.
                    //
                    p.IcePing();
                    Connection con = p.GetConnection()!;
                    var cb = new CallbackBase();
                    Task t = p.startDispatchAsync(
                        progress: new Progress(sentSynchronously => cb.Called()));
                    cb.Check(); // Ensure the request was sent before we close the connection.
                    con.Close(ConnectionClose.Forcefully);
                    try
                    {
                        t.Wait();
                        TestHelper.Assert(false);
                    }
                    catch (AggregateException ex)
                    {
                        TestHelper.Assert(ex.InnerException is ConnectionClosedLocallyException);
                    }
                    p.finishDispatch();

                    //
                    // Remote case: the server closes the connection forcefully. This causes the request to fail
                    // with a ConnectionLostException. Since the close() operation is not idempotent, the client
                    // will not retry.
                    //
                    try
                    {
                        p.close(CloseMode.Forcefully);
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
                var q = Outer.Inner.ITestIntfPrx.Parse($"test2:{helper.GetTestEndpoint(0)}", communicator);
                q.opAsync(1).ContinueWith(t =>
                    {
                        (int ReturnValue, int j) = t.Result;
                        TestHelper.Assert(ReturnValue == 1);
                        TestHelper.Assert(j == 1);
                    }).Wait();
            }
            output.WriteLine("ok");

            p.shutdown();
        }
    }
}
