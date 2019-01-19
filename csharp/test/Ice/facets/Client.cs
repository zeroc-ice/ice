//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice
{
    namespace facets
    {
        public class Client : TestHelper
        {
            public override void run(string[] args)
            {
                using(var communicator = initialize(ref args))
                {
                    var g = AllTests.allTests(this);
                    g.shutdown();
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Client>(args);
            }
        }
    }
}
