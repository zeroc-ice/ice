//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice
{
    namespace inheritance
    {
        public class Collocated : TestHelper
        {
            public override void run(string[] args)
            {
                using(var communicator = initialize(ref args))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    var adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.add(new InitialI(adapter), Ice.Util.stringToIdentity("initial"));
                    AllTests.allTests(this);
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Collocated>(args);
            }
        }
    }
}
