//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Threading
{
    public class Server : TestHelper
    {
        public override void Run(string[] args)
        {
            using var communicator = Initialize(ref args);

            var adapter = communicator.CreateObjectAdapterWithEndpoints("TestAdapter", GetTestEndpoint(0));
            adapter.Add("test", new TestIntf(TaskScheduler.Default));
            adapter.Activate();

            var schedulerPair = new ConcurrentExclusiveSchedulerPair(TaskScheduler.Default, 5);

            var adapter2 = communicator.CreateObjectAdapterWithEndpoints("TestAdapterExclusiveTS", GetTestEndpoint(1),
                taskScheduler: schedulerPair.ExclusiveScheduler);
            adapter2.Add("test", new TestIntf(schedulerPair.ExclusiveScheduler));
            adapter2.Activate();

            var adapter3 = communicator.CreateObjectAdapterWithEndpoints("TestAdapteConcurrentTS", GetTestEndpoint(2),
                taskScheduler: schedulerPair.ConcurrentScheduler);
            adapter3.Add("test", new TestIntf(schedulerPair.ConcurrentScheduler));
            adapter3.Activate();

            // Setup 20 worker threads for the .NET thread pool (we setup the minimum to avoid delays from the
            // thread pool thread creation).
            // TODO: Why are worker threads used here and not completion port threads? The SocketAsyncEventArgs
            // Completed event handler is called from the worker thread and not the completion port thread. This
            // might require fixing once we use Async socket primitives.
            ThreadPool.SetMinThreads(20, 4);
            ThreadPool.SetMaxThreads(20, 4);

            ServerReady();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
    }
}
