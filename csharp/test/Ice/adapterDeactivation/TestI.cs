// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.AdapterDeactivation
{
    public sealed class TestIntf : ITestIntf
    {
        public void Transient(Current current, CancellationToken cancel)
        {
            bool ice1 = TestHelper.GetTestProtocol(current.Communicator.GetProperties()) == Protocol.Ice1;
            var transport = TestHelper.GetTestTransport(current.Communicator.GetProperties());
            var endpoint = ice1 ? $"{transport} -h \"::0\"" : $"ice+{transport}://[::0]:0";

            using ObjectAdapter adapter =
                current.Communicator.CreateObjectAdapterWithEndpoints("TransientTestAdapter", endpoint);
            adapter.Activate();
        }

        public void Deactivate(Current current, CancellationToken cancel)
        {
            _ = current.Adapter.DisposeAsync().AsTask();
            Thread.Sleep(100);
            Task.Delay(100, cancel).ContinueWith(t => current.Communicator.ShutdownAsync(), TaskScheduler.Current);
        }
    }
}
