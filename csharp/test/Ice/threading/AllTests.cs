// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Threading
{
    public static class AllTests
    {
        private class Progress : IProgress<bool>
        {
            public TaskScheduler Scheduler
            {
                get
                {
                    _event.WaitOne();
                    return _scheduler!;
                }
            }

            private TaskScheduler? _scheduler;

            private readonly ManualResetEvent _event = new(false);

            void IProgress<bool>.Report(bool value)
            {
                _scheduler = TaskScheduler.Current;
                _event.Set();
            }
        }
        public static async ValueTask AllTestsWithServer(TestHelper helper, bool collocated, int server)
        {
            System.IO.TextWriter output = helper.Output;

            TaskScheduler? scheduler = TaskScheduler.Current;

            var proxy = ITestIntfPrx.Parse(helper.GetTestProxy("test", server), helper.Communicator!);

            if (collocated)
            {
                // With collocation, synchronous calls dispatched on an object adapter which doesn't set a task
                // scheduler are dispatched from the client invocation task scheduler.
                var context = new Dictionary<string, string>() { { "scheduler", scheduler.Id.ToString() } };
                proxy.PingSync(context);
                proxy.Ping(context);
            }
            else
            {
                proxy.PingSync();
                proxy.Ping();
            }

            // Ensure the continuation is ran on the current task scheduler
            await proxy.PingSyncAsync();
            TestHelper.Assert(TaskScheduler.Current == scheduler);
            await proxy.PingAsync();
            TestHelper.Assert(TaskScheduler.Current == scheduler);

            // The continuation set with ContinueWith are expected to be ran on the Current task
            // scheduler if no task scheduler is provided to ContinueWith.
            Func<string, Action<Task>> checkScheduler = (op) =>
            {
                return t =>
                {
                    if (TaskScheduler.Current != scheduler)
                    {
                        throw new TestFailedException(
                            $"unexpected scheduler for {op} ContinueWith {TaskScheduler.Current}");
                    }
                };
            };
            await proxy.PingSyncAsync().ContinueWith(checkScheduler("pingSyncAsync"), TaskScheduler.Current);
            TestHelper.Assert(TaskScheduler.Current == scheduler);
            await proxy.PingAsync().ContinueWith(checkScheduler("pingAsyncAsync"), TaskScheduler.Current);
            TestHelper.Assert(TaskScheduler.Current == scheduler);

            // The progress Report callback is from the default task scheduler or the caller's thread scheduler.
            Progress progress;
            progress = new Progress();
            await proxy.PingSyncAsync(progress: progress);
            TestHelper.Assert(progress.Scheduler == TaskScheduler.Default || progress.Scheduler == scheduler);
            progress = new Progress();
            await proxy.PingAsync(progress: progress);
            TestHelper.Assert(progress.Scheduler == TaskScheduler.Default || progress.Scheduler == scheduler);

            // The continuation of an awaitable setup with ConfigureAwait(false) is ran with the default
            // scheduler unless it completes synchronously in which cause it will run on the current
            // scheduler.
            await proxy.PingSyncAsync().ConfigureAwait(false);
            TestHelper.Assert(TaskScheduler.Current == TaskScheduler.Default);
            await proxy.PingAsync().ConfigureAwait(false);
            TestHelper.Assert(TaskScheduler.Current == TaskScheduler.Default);
        }

        public static async Task RunAsync(TestHelper helper, bool collocated)
        {
            Communicator communicator = helper.Communicator!;

            var schedulers = new ConcurrentExclusiveSchedulerPair(TaskScheduler.Default, 2);
            Dictionary<string, string> properties = communicator.GetProperties();
            System.IO.TextWriter output = helper.Output;

            // Use the Default task scheduler to run continuations tests with the 3 object adapters
            // setup by the server, each object adapter uses a different task scheduler.
            output.Write("testing continuations with default task scheduler... ");
            {
                await AllTestsWithServer(helper, collocated, 0); // Test with server endpoint 0
                await AllTestsWithServer(helper, collocated, 1); // Test with server endpoint 1
                await AllTestsWithServer(helper, collocated, 2); // Test with server endpoint 2
            }
            output.WriteLine("ok");

            // Use the concurrent task scheduler
            output.Write("testing continuations with concurrent task scheduler... ");
            await Task.Factory.StartNew(async () => await AllTestsWithServer(helper, collocated, 0),
                                        default,
                                        TaskCreationOptions.None, schedulers.ConcurrentScheduler).Unwrap();
            output.WriteLine("ok");

            // Use the exclusive task scheduler
            output.Write("testing continuations with exclusive task scheduler... ");
            await Task.Factory.StartNew(async () => await AllTestsWithServer(helper, collocated, 0),
                                        default,
                                        TaskCreationOptions.None, schedulers.ExclusiveScheduler).Unwrap();
            output.WriteLine("ok");

            output.Write("testing server-side default task scheduler concurrency... ");
            {
                // With the default task scheduler, the concurrency is limited to the number of .NET thread pool
                // threads. The server sets up at least 20 threads in the .NET Thread pool we test this level
                // of concurrency but in theory it could be much higher.
                var proxy = ITestIntfPrx.Parse(helper.GetTestProxy("test", 0), communicator);
                try
                {
                    Task.WaitAll(Enumerable.Range(0, 25).Select(idx => proxy.ConcurrentAsync(20)).ToArray());
                }
                catch (AggregateException ex)
                {
                    // On Windows, it's not uncommon that the .NET thread pool creates one or two additional threads
                    // and doesn't strictly respect the number of configured maximum threads. So we tolerate at least
                    // 4 additional concurrent calls.
                    TestHelper.Assert(ex.InnerExceptions.Count < 5);
                }
                proxy.Reset();
            }
            output.WriteLine("ok");

            output.Write("testing server-side exclusive task scheduler... ");
            {
                // With the exclusive task scheduler, at most one request can be dispatched concurrently.
                var proxy = ITestIntfPrx.Parse(helper.GetTestProxy("test", 1), communicator);
                Task.WaitAll(Enumerable.Range(0, 10).Select(idx => proxy.ConcurrentAsync(1)).ToArray());
            }
            output.WriteLine("ok");

            output.Write("testing server-side concurrent task scheduler... ");
            {
                // With the concurrent task scheduler, at most 5 requests can be dispatched concurrently (this is
                // configured on the server side).
                var proxy = ITestIntfPrx.Parse(helper.GetTestProxy("test", 2), communicator);
                await Task.WhenAll(Enumerable.Range(0, 20).Select(idx => proxy.ConcurrentAsync(5)).ToArray());
            }
            output.WriteLine("ok");

            await ITestIntfPrx.Parse(helper.GetTestProxy("test", 0), communicator).ShutdownAsync();
        }
    }
}
