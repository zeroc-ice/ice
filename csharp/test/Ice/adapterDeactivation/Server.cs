//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace Ice
{
    namespace adapterDeactivation
    {
        public class Server : TestHelper
        {
            public override void run(string[] args)
            {
                using(var communicator = initialize(ref args))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    Ice.ServantLocator locator = new ServantLocatorI();
                    adapter.addServantLocator(locator, "");
                    adapter.activate();
                    serverReady();
                    adapter.waitForDeactivate();
                }
            }

            public static Task<int> Main(string[] args) =>
                TestDriver.runTestAsync<Server>(args);
        }
    }
}
