//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Linq;
using Test;

namespace Ice
{
    namespace invoke
    {
        public class ServantLocatorI : Ice.ServantLocator
        {
            public ServantLocatorI()
            {
                _blobject = new BlobjectI();
            }

            public Ice.Disp
            locate(Ice.Current current, out object cookie)
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

        public class ServantLocatorAsyncI : Ice.ServantLocator
        {
            public ServantLocatorAsyncI()
            {
                _blobject = new BlobjectAsyncI();
            }

            public Ice.Disp
            locate(Ice.Current current, out object cookie)
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
                using (var communicator = initialize(ref args))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    if (async)
                    {
                        adapter.AddServantLocator(new ServantLocatorAsyncI(), "");
                    }
                    else
                    {
                        adapter.AddServantLocator(new ServantLocatorI(), "");
                    }
                    adapter.Activate();
                    serverReady();
                    communicator.waitForShutdown();
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Server>(args);
            }
        }
    }
}
