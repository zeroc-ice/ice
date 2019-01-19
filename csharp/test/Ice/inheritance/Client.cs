//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice
{
    namespace inheritance
    {
        public class Client : TestHelper
        {
            public override void run(string[] args)
            {
                using(var communicator = initialize(ref args))
                {
                    var initial = AllTests.allTests(this);
                    initial.shutdown();
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Client>(args);
            }
        }
    }
}
