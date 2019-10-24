//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace binding
    {
        public class RemoteCommunicatorI : Test.RemoteCommunicatorDisp_
        {
            public override Test.RemoteObjectAdapterPrx
            createObjectAdapter(string name, string endpts, Ice.Current current)
            {
                int retry = 5;
                while (true)
                {
                    try
                    {
                        Ice.Communicator communicator = current.adapter.getCommunicator();
                        string endpoints = endpts;
                        if (endpoints.IndexOf("-p") < 0)
                        {
                            endpoints = global::Test.TestHelper.getTestEndpoint(communicator.getProperties(), _nextPort++, endpoints);
                        }

                        communicator.getProperties().setProperty(name + ".ThreadPool.Size", "1");
                        Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints(name, endpoints);
                        return Test.RemoteObjectAdapterPrxHelper.uncheckedCast(
                            current.adapter.addWithUUID(new RemoteObjectAdapterI(adapter)));
                    }
                    catch (Ice.SocketException)
                    {
                        if (--retry == 0)
                        {
                            throw;
                        }
                    }
                }
            }

            public override void
            deactivateObjectAdapter(Test.RemoteObjectAdapterPrx adapter, Ice.Current current)
            {
                adapter.deactivate(); // Collocated call.
            }

            public override void
            shutdown(Ice.Current current)
            {
                current.adapter.getCommunicator().shutdown();
            }

            private int _nextPort = 10;
        }
    }
}
