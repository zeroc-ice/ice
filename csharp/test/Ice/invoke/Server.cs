// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.invoke
{
        public class ServantLocatorI : Ice.ServantLocator
        {
            public ServantLocatorI(bool async)
            {
                if (async)
                {
                    _blobject = new BlobjectAsyncI();
                }
                else
                {
                    _blobject = new BlobjectI();
                }
            }

            public Ice.Object
            locate(Ice.Current current, out System.Object cookie)
            {
                cookie = null;
                return _blobject;
            }

            public void
            finished(Ice.Current current, Ice.Object servant, System.Object cookie)
            {
            }

            public void
            deactivate(string category)
            {
            }

            private Ice.Object _blobject;
        }

        public class Server : TestHelper
        {
            public override void run(string[] args)
            {
                bool async = args.Any(v => v == "--async");
                using (var communicator = initialize(ref args))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.addServantLocator(new ServantLocatorI(async), "");
                    adapter.activate();
                    serverReady();
                    communicator.waitForShutdown();
                }
            }

            public static Task<int> Main(string[] args) =>
                TestDriver.runTestAsync<Server>(args);
        }
    }

