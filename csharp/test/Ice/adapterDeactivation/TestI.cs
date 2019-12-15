//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace adapterDeactivation
    {
        public sealed class TestI : Test.TestIntf
        {
            public void transient(Ice.Current current)
            {
                Ice.Communicator communicator = current.Adapter.Communicator;

                Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
                adapter.Activate();
                adapter.Destroy();
            }

            public void deactivate(Ice.Current current)
            {
                current.Adapter.Deactivate();
                System.Threading.Thread.Sleep(100);
            }
        }
    }
}
