//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

namespace ZeroC.Ice.Test.AdapterDeactivation
{
    public sealed class TestIntf : ITestIntf
    {
        public void Transient(Current current)
        {
            using ObjectAdapter adapter = current.Adapter.Communicator.CreateObjectAdapterWithEndpoints(
                "TransientTestAdapter", "default");
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
