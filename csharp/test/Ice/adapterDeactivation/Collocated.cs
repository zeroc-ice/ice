// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.adapterDeactivation
{


        public class Collocated : TestHelper
        {
            public override async Task runAsync(string[] args)
            {
                using (var communicator = initialize(ref args))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));

                    //
                    // 2 threads are necessary to dispatch the collocated transient() call with AMI
                    //
                    communicator.getProperties().setProperty("TestAdapter.ThreadPool.Size", "2");

                    var adapter = communicator.createObjectAdapter("TestAdapter");
                    var locator = new ServantLocatorI();
                    adapter.addServantLocator(locator, "");

                    await AllTests.allTests(this);

                    adapter.waitForDeactivate();
                }
            }

            public static Task<int> Main(string[] args) =>
                TestDriver.runTestAsync<Collocated>(args);
        }
    }

