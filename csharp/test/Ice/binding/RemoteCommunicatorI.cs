// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using Test;

namespace ZeroC.Ice.Test.Binding
{
    public class RemoteCommunicator : IRemoteCommunicator
    {
        private int _nextPort = 10;

        public IRemoteObjectAdapterPrx CreateObjectAdapter(
            string name,
            string transport,
            Current current,
            CancellationToken cancel)
        {
            int retry = 5;
            while (true)
            {
                try
                {
                    string endpoints =
                        TestHelper.GetTestEndpoint(current.Communicator.GetProperties(), _nextPort++, transport);

                    if (transport == "udp")
                    {
                        // udp endpoints are always non-secure. Set name.AcceptNonSecure = Always
                        current.Communicator.SetProperty($"{name}.AcceptNonSecure", "Always");
                    }
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

        public IRemoteObjectAdapterPrx CreateObjectAdapterWithEndpoints(
            string name,
            string endpoints,
            Current current,
            CancellationToken cancel)
        {
            ObjectAdapter adapter = current.Communicator.CreateObjectAdapterWithEndpoints(name, endpoints);
            return current.Adapter.AddWithUUID(new RemoteObjectAdapter(adapter), IRemoteObjectAdapterPrx.Factory);
        }

        // Collocated call.
        public void DeactivateObjectAdapter(
            IRemoteObjectAdapterPrx? adapter,
            Current current,
            CancellationToken cancel) =>
            adapter!.Deactivate(cancel: cancel);

        public void Shutdown(Current current, CancellationToken cancel) =>
            current.Communicator.ShutdownAsync();
    }
}
