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

        public IObject
        Locate(Current current, out object cookie)
        {
            cookie = null;
            return _blobject;
        }

        public void
        Finished(Current current, IObject servant, object cookie)
        {
        }

        public void
        Deactivate(string category)
        {
        }

        private Blobject _blobject;
    }

    public class ServantLocatorAsync : IServantLocator
    {
        public ServantLocatorAsync() => _blobject = new BlobjectAsyncI();

        public IObject
        Locate(Current current, out object cookie)
        {
            cookie = null;
            return _blobject;
        }

        public void
        Finished(Current current, IObject servant, object cookie)
        {
        }

        public void
        Deactivate(string category)
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
