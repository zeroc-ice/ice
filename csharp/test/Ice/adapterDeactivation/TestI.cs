//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.AdapterDeactivation
{
    public sealed class TestIntf : ITestIntf
    {
        public void Transient(Current current)
        {
            bool ice1 = TestHelper.GetTestProtocol(current.Adapter.Communicator.GetProperties()) == Protocol.Ice1;
            var transport = TestHelper.GetTestTransport(current.Adapter.Communicator.GetProperties());
            var endpoint = ice1 ? "{transport} -h localhost" : $"ice+{transport}://localhost:0";

            using ObjectAdapter adapter = current.Adapter.Communicator.CreateObjectAdapterWithEndpoints(
                "TransientTestAdapter", endpoint);
            adapter.Activate();
        }

        public void Deactivate(Current current)
        {
            _ = current.Adapter.DisposeAsync().AsTask();
            System.Threading.Thread.Sleep(100);
            Task.Delay(100).ContinueWith(t => current.Communicator.ShutdownAsync(), TaskScheduler.Current);
        }
    }
}
