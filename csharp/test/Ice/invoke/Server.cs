// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Linq;
using Test;

namespace Ice
{
    namespace invoke
    {
        public class ServantLocatorI : Ice.ServantLocator
        {
            public ServantLocatorI(bool async)
            {
                if(async)
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
                bool async = args.Any(v => v.Equals("--async"));
                using(var communicator = initialize(ref args))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.addServantLocator(new ServantLocatorI(async), "");
                    adapter.activate();
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
