//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.Binding
{
    public class RemoteCommunicator : IRemoteCommunicator
    {
        public IRemoteObjectAdapterPrx CreateObjectAdapter(string name, string transport, Current current)
        {
            int retry = 5;
            while (true)
            {
                try
                {
                    string endpoints =
                        TestHelper.GetTestEndpoint(current.Communicator.GetProperties(), _nextPort++, transport);

                    ObjectAdapter adapter = current.Communicator.CreateObjectAdapterWithEndpoints(name, endpoints);
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

        public IRemoteObjectAdapterPrx CreateObjectAdapterWithEndpoints(string name, string endpoints, Current current)
        {
            ObjectAdapter adapter = current.Communicator.CreateObjectAdapterWithEndpoints(name, endpoints);
            return current.Adapter.AddWithUUID(new RemoteObjectAdapter(adapter), IRemoteObjectAdapterPrx.Factory);
        }

        // Collocated call.
        public void DeactivateObjectAdapter(IRemoteObjectAdapterPrx? adapter, Current current) =>
            adapter!.Deactivate();

        public void Shutdown(Current current) => current.Adapter.Communicator.ShutdownAsync();

        private int _nextPort = 10;
    }
}
