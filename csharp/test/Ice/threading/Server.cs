// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Threading
{
    public class Server : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            await Communicator.ActivateAsync();

            ObjectAdapter? adapter = Communicator.CreateObjectAdapterWithEndpoints("TestAdapter", GetTestEndpoint(0));
            adapter.Add("test", new TestIntf(TaskScheduler.Default));
            await adapter.ActivateAsync();

            var schedulerPair = new ConcurrentExclusiveSchedulerPair(TaskScheduler.Default, 5);

            ObjectAdapter? adapter2 = Communicator.CreateObjectAdapterWithEndpoints(
                "TestAdapterExclusiveTS",
                GetTestEndpoint(1),
                taskScheduler: schedulerPair.ExclusiveScheduler);
            adapter2.Add("test", new TestIntf(schedulerPair.ExclusiveScheduler));
            await adapter2.ActivateAsync();

            ObjectAdapter? adapter3 = Communicator.CreateObjectAdapterWithEndpoints(
                "TestAdapteConcurrentTS",
                GetTestEndpoint(2),
                taskScheduler: schedulerPair.ConcurrentScheduler);
            adapter3.Add("test", new TestIntf(schedulerPair.ConcurrentScheduler));
            await adapter3.ActivateAsync();

            // Setup 20 worker threads for the .NET thread pool (we setup the minimum to avoid delays from the
            // thread pool thread creation).
            // TODO: Why are worker threads used here and not completion port threads? The SocketAsyncEventArgs
            // Completed event handler is called from the worker thread and not the completion port thread. This
            // might require fixing once we use Async socket primitives.
            ThreadPool.SetMinThreads(20, 4);
            ThreadPool.SetMaxThreads(20, 4);

            ServerReady();
            await Communicator.ShutdownComplete;
        }

        public static async Task<int> Main(string[] args)
        {
            await using var communicator = CreateCommunicator(ref args);
            return await RunTestAsync<Server>(communicator, args);
        }
    }
}
