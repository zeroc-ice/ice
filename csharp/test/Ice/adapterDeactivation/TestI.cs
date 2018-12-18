// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    namespace adapterDeactivation
    {
        public sealed class TestI : Test.TestIntfDisp_
        {
            public override void transient(Ice.Current current)
            {
                Ice.Communicator communicator = current.adapter.getCommunicator();

                Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
                adapter.activate();
                adapter.destroy();
            }

            public override void deactivate(Ice.Current current)
            {
                current.adapter.deactivate();
                System.Threading.Thread.Sleep(100);
            }
        }
    }
}
