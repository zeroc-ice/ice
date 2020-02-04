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
                        endpoints = global::Test.TestHelper.getTestEndpoint(communicator.GetProperties(), _nextPort++, endpoints);
                    }

                    communicator.SetProperty(name + ".ThreadPool.Size", "1");
                    ObjectAdapter adapter = communicator.CreateObjectAdapterWithEndpoints(name, endpoints);
                    return current.Adapter.AddWithUUID(
                        new RemoteObjectAdapter(adapter), IRemoteObjectAdapterPrx.Factory);
                }
                catch (SocketException)
                {
                    if (--retry == 0)
                    {
                        throw;
                    }
                }
            }
        }

        public void
        deactivateObjectAdapter(IRemoteObjectAdapterPrx adapter, Current current) => adapter.deactivate(); // Collocated call.

        public void
        shutdown(Current current) => current.Adapter.Communicator.Shutdown();

        private int _nextPort = 10;
    }
}
