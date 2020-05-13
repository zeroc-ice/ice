//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice.binding.Test;

namespace Ice.binding
{
    public class RemoteCommunicator : IRemoteCommunicator
    {
        public IRemoteObjectAdapterPrx
        createObjectAdapter(string name, string endpts, Current current)
        {
            int retry = 5;
            while (true)
            {
                try
                {
                    Communicator communicator = current.Adapter.Communicator;
                    string endpoints = endpts;
                    if (endpoints.IndexOf("-p") < 0)
                    {
                        endpoints = global::Test.TestHelper.GetTestEndpoint(communicator.GetProperties(), _nextPort++, endpoints);
                    }

                    ObjectAdapter adapter = communicator.CreateObjectAdapterWithEndpoints(name, endpoints);
                    return current.Adapter.AddWithUUID(
                        new RemoteObjectAdapter(adapter), IRemoteObjectAdapterPrx.Factory);
                }
                catch (TransportException)
                {
                    if (--retry == 0)
                    {
                        throw;
                    }
                }
            }
        }

        // Collocated call.
        public void deactivateObjectAdapter(IRemoteObjectAdapterPrx? adapter, Current current) =>
            adapter!.deactivate();

        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();

        private int _nextPort = 10;
    }
}
