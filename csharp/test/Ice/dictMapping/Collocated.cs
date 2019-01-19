//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice
{
    namespace dictMapping
    {
        public class Collocated : TestHelper
        {
            public override void run(string[] args)
            {
                using(var communicator = initialize(ref args))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    var adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.add(new MyClassI(), Ice.Util.stringToIdentity("test"));
                    //adapter.activate(); // Don't activate OA to ensure collocation is used.
                    AllTests.allTests(this, true);
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Collocated>(args);
            }
        }
    }
}
