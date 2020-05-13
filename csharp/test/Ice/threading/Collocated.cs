//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.threading.Test;
using System;
using System.Threading;
using System.Threading.Tasks;

namespace Ice.threading
{
    public class Collocated : TestHelper
    {
        public override void Run(string[] args)
        {
            using var communicator = Initialize(ref args);

            var adapter1 = communicator.CreateObjectAdapterWithEndpoints("TestAdapter", GetTestEndpoint(0));
            adapter1.Add("test", new TestIntf(TaskScheduler.Default));

            var schedulerPair = new ConcurrentExclusiveSchedulerPair(TaskScheduler.Default, 5);

            var adapter2 = communicator.CreateObjectAdapterWithEndpoints("TestAdapterExclusiveTS", GetTestEndpoint(1),
                taskScheduler: schedulerPair.ExclusiveScheduler);
            adapter2.Add("test", new TestIntf(schedulerPair.ExclusiveScheduler));

            var adapter3 = communicator.CreateObjectAdapterWithEndpoints("TestAdapteConcurrentTS", GetTestEndpoint(2),
                taskScheduler: schedulerPair.ConcurrentScheduler);
            adapter3.Add("test", new TestIntf(schedulerPair.ConcurrentScheduler));

            // Setup 21 worker threads for the .NET thread pool (we setup the minimum to avoid delays from the
            // thread pool thread creation). Unlike the server we setup one additional thread for running the
            // allTests task in addition to the 20 concurrent threads which are needed for concurrency testing.
            // TODO: Why are worker threads used here and not completion port threads? The SocketAsyncEventArgs
            // Completed event handler is called from the worker thread and not the completion port thread.
            ThreadPool.SetMinThreads(21, 4);
            ThreadPool.SetMaxThreads(21, 4);

            try
            {
                AllTests.allTests(this, true).AsTask().Wait();
            }
            catch (AggregateException ex)
            {
                if (ex.InnerException is TestFailedException failedEx)
                {
                    GetWriter().WriteLine($"test failed: {failedEx.reason}");
                    Assert(false);
                }
                throw;
            }
            catch (TestFailedException ex)
            {
                GetWriter().WriteLine($"test failed: {ex.reason}");
                Assert(false);
            }
        }

        public static int Main(string[] args) => TestDriver.RunTest<Collocated>(args);
    }
}
