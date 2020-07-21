//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.Binding
{
    public class RemoteCommunicator : IRemoteCommunicator
    {
        public IRemoteObjectAdapterPrx CreateObjectAdapter(string name, string endpts, Current current)
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
                        endpoints = TestHelper.GetTestEndpoint(communicator.GetProperties(), _nextPort++, endpoints);
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
        public void DeactivateObjectAdapter(IRemoteObjectAdapterPrx? adapter, Current current) =>
            adapter!.Deactivate();

        public void Shutdown(Current current) => current.Adapter.Communicator.ShutdownAsync();

        private int _nextPort = 10;
    }
}
