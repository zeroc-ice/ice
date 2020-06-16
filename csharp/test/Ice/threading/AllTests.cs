//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Threading
{
    public class AllTests
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

            private ManualResetEvent _event = new ManualResetEvent(false);

            void IProgress<bool>.Report(bool value)
            {
                _scheduler = TaskScheduler.Current;
                _event.Set();
            }
        }
        public static async ValueTask allTestsWithServer(TestHelper helper, bool collocated, int server)
        {
            System.IO.TextWriter output = helper.GetWriter();

            var scheduler = TaskScheduler.Current;

            var proxy = ITestIntfPrx.Parse("test:" + helper.GetTestEndpoint(server), helper.Communicator()!);

            if (collocated)
            {
                // With collocation, synchronous calls dispatched on an object adapter which doesn't set a task
                // scheduler are dispatched from the client invocation task scheduler.
                var context = new Dictionary<string, string>() { { "scheduler", scheduler.Id.ToString() } };
                proxy.pingSync(context);
                proxy.ping(context);
            }
            else
            {
                proxy.pingSync();
                proxy.ping();
            }

            // Ensure the continuation is ran on the current task scheduler
            await proxy.pingSyncAsync();
            TestHelper.Assert(TaskScheduler.Current == scheduler);
            await proxy.pingAsync();
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
            await proxy.pingSyncAsync().ContinueWith(checkScheduler("pingSyncAsync"));
            TestHelper.Assert(TaskScheduler.Current == scheduler);
            await proxy.pingAsync().ContinueWith(checkScheduler("pingAsyncAsync"));
            TestHelper.Assert(TaskScheduler.Current == scheduler);

            // The progress Report callback is always called from the default task scheduler right now.
            Progress progress;
            progress = new Progress();
            await proxy.pingSyncAsync(progress: progress);
            TestHelper.Assert(progress.Scheduler == TaskScheduler.Default);
            progress = new Progress();
            await proxy.pingAsync(progress: progress);
            TestHelper.Assert(progress.Scheduler == TaskScheduler.Default);

            // The continuation of an awaitable setup with ConfigureAwait(false) is ran with the default
            // scheduler unless it completes synchronously in which cause it will run on the current
            // scheduler.
            await proxy.pingSyncAsync().ConfigureAwait(false);
            TestHelper.Assert(TaskScheduler.Current == TaskScheduler.Default);
            await proxy.pingAsync().ConfigureAwait(false);
            TestHelper.Assert(TaskScheduler.Current == TaskScheduler.Default);
        }

        public static async ValueTask<ITestIntfPrx> allTests(TestHelper helper, bool collocated)
        {
            Communicator communicator = helper.Communicator()!;
            TestHelper.Assert(communicator != null);

            var schedulers = new ConcurrentExclusiveSchedulerPair(TaskScheduler.Default, 2);
            Dictionary<string, string> properties = communicator.GetProperties();
            System.IO.TextWriter output = helper.GetWriter();

            // Use the Default task scheduler to run continuations tests with the 3 object adpaters
            // setup by the server, each object adapter uses a different task scheduler.
            output.Write("testing continuations with default task scheduler... ");
            {
                await allTestsWithServer(helper, collocated, 0); // Test with server endpoint 0
                await allTestsWithServer(helper, collocated, 1); // Test with server endpoint 1
                await allTestsWithServer(helper, collocated, 2); // Test with server endpoint 2
            }
            output.WriteLine("ok");

            // Use the concurrent task scheduler
            output.Write("testing continuations with concurrent task scheduler... ");
            await Task.Factory.StartNew(async () =>
            {
                await allTestsWithServer(helper, collocated, 0);
            }, default, TaskCreationOptions.None, schedulers.ConcurrentScheduler).Unwrap();
            output.WriteLine("ok");

            // Use the exclusive task scheduler
            output.Write("testing continuations with exclusive task scheduler... ");
            await Task.Factory.StartNew(async () =>
            {
                await allTestsWithServer(helper, collocated, 0);
            }, default, TaskCreationOptions.None, schedulers.ExclusiveScheduler).Unwrap();
            output.WriteLine("ok");

            output.Write("testing server-side default task scheduler concurrency... ");
            {
                // With the default task scheduler, the concurrency is limited to the number of .NET thread pool
                // threads. The server sets up at least 20 threads in the .NET Thread pool we test this level
                // of concurrency but in theory it could be much higher.
                var proxy = ITestIntfPrx.Parse("test:" + helper.GetTestEndpoint(0), communicator);
                try
                {
                    Task.WaitAll(Enumerable.Range(0, 25).Select(idx => proxy.concurrentAsync(20)).ToArray());
                }
                catch (AggregateException ex)
                {
                    // On Windows, it's not un-common that the .NET thread pool creates one or two additional threads
                    // and doesn't striclty respect the number of configured maximum threads. So we tolerate a lest
                    // 2 additional concurrent calls.
                    TestHelper.Assert(ex.InnerExceptions.Count < 3);
                }
                proxy.reset();
            }
            output.WriteLine("ok");

            output.Write("testing server-side exclusive task scheduler... ");
            {
                // With the exclusive task scheduler, at most one request can be dispatched concurrently.
                var proxy = ITestIntfPrx.Parse("test:" + helper.GetTestEndpoint(1), communicator);
                Task.WaitAll(Enumerable.Range(0, 10).Select(idx => proxy.concurrentAsync(1)).ToArray());
            }
            output.WriteLine("ok");

            output.Write("testing server-side concurrent task scheduler... ");
            {
                // With the concurrent task scheduler, at most 5 requests can be dispatched concurrently (this is
                // configured on the server side).
                var proxy = ITestIntfPrx.Parse("test:" + helper.GetTestEndpoint(2), communicator);
                Task.WaitAll(Enumerable.Range(0, 20).Select(idx => proxy.concurrentAsync(5)).ToArray());
            }
            output.WriteLine("ok");

            return ITestIntfPrx.Parse("test:" + helper.GetTestEndpoint(0), communicator);
        }
    }
}
