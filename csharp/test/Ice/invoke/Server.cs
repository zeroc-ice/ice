//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Linq;
using Test;

namespace Ice.invoke
{
    public class ServantLocator : IServantLocator
    {
        public ServantLocator() => _blobject = new BlobjectI();

        public Disp
        locate(Current current, out object cookie)
        {
            cookie = null;
            return (current, incoming) => _blobject.Dispatch(current, incoming);
        }

        public void
        finished(Current current, Disp servant, object cookie)
        {
        }

        public void
        deactivate(string category)
        {
        }

        private Blobject _blobject;
    }

    public class ServantLocatorAsync : IServantLocator
    {
        public ServantLocatorAsync() => _blobject = new BlobjectAsyncI();

        public Disp
        locate(Current current, out object cookie)
        {
            cookie = null;
            return (current, incoming) => _blobject.Dispatch(current, incoming);
        }

        public void
        finished(Current current, Disp servant, object cookie)
        {
        }

        public void
        deactivate(string category)
        {
        }

        private BlobjectAsyncI _blobject;
    }

    public class Server : TestHelper
    {
        public override void run(string[] args)
        {
            bool async = args.Any(v => v.Equals("--async"));
            using var communicator = initialize(ref args);
            communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            if (async)
            {
                adapter.AddServantLocator(new ServantLocatorAsync(), "");
            }
            else
            {
                adapter.AddServantLocator(new ServantLocator(), "");
            }
            adapter.Activate();
            serverReady();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.runTest<Server>(args);
    }
}
