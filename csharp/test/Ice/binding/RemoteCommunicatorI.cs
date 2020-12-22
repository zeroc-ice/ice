// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Binding
{
    public class RemoteCommunicator : IAsyncRemoteCommunicator
    {
        private int _nextPort = 10;

        public async ValueTask<IRemoteObjectAdapterPrx> CreateObjectAdapterAsync(
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
                    await adapter.ActivateAsync(cancel);

                    return current.Adapter.AddWithUUID(new RemoteObjectAdapter(adapter),
                                                       IRemoteObjectAdapterPrx.Factory);
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

        public async ValueTask<IRemoteObjectAdapterPrx> CreateObjectAdapterWithEndpointsAsync(
            string name,
            string endpoints,
            Current current,
            CancellationToken cancel)
        {
            ObjectAdapter adapter = current.Communicator.CreateObjectAdapterWithEndpoints(name, endpoints);
            await adapter.ActivateAsync(cancel);

            return current.Adapter.AddWithUUID(new RemoteObjectAdapter(adapter), IRemoteObjectAdapterPrx.Factory);
        }

        // Colocated call.
        public ValueTask DeactivateObjectAdapterAsync(
            IRemoteObjectAdapterPrx adapter,
            Current current,
            CancellationToken cancel) =>
            new(adapter.DeactivateAsync(cancel: cancel));

        public ValueTask ShutdownAsync(Current current, CancellationToken cancel)
        {
            _ = current.Communicator.ShutdownAsync(); // only initiate shutdown
            return default;
        }
    }
}
