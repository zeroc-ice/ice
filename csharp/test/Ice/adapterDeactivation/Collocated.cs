//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice
{
    namespace adapterDeactivation
    {
        public class Collocated : TestHelper
        {
            public override void run(string[] args)
            {
                using (var communicator = initialize(ref args))
                {
                    communicator.Properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(0));

                    //
                    // 2 threads are necessary to dispatch the collocated transient() call with AMI
                    //
                    communicator.Properties.setProperty("TestAdapter.ThreadPool.Size", "2");

                    var adapter = communicator.createObjectAdapter("TestAdapter");
                    var locator = new ServantLocatorI();
                    adapter.AddServantLocator(locator, "");

                    AllTests.allTests(this);

                    adapter.WaitForDeactivate();
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Collocated>(args);
            }
        }
    }
}
