//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

namespace ZeroC.Ice.Test.AdapterDeactivation
{
    public sealed class TestIntf : ITestIntf
    {
        public void transient(Current current)
        {
            using ObjectAdapter adapter = current.Adapter.Communicator.CreateObjectAdapterWithEndpoints(
                "TransientTestAdapter", "default");
            _ = adapter.ActivateAsync();
        }

        public void deactivate(Current current)
        {
            _ = current.Adapter.DeactivateAsync();
            System.Threading.Thread.Sleep(100);
            Task.Delay(100).ContinueWith(t => current.Communicator.ShutdownAsync());
        }
    }
}
