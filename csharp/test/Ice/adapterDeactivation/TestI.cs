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
                Ice.Communicator communicator = current.adapter.getCommunicator();

                Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
                adapter.activate();
                adapter.destroy();
            }

            public void deactivate(Ice.Current current)
            {
                current.adapter.deactivate();
                System.Threading.Thread.Sleep(100);
            }
        }
    }
}
