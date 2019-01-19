//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice
{
    namespace invoke
    {
        public class Client : global::Test.TestHelper
        {
            public override void run(string[] args)
            {
                using(var communicator = initialize(ref args))
                {
                    var myClass = AllTests.allTests(this);
                    myClass.shutdown();
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Client>(args);
            }
        }
    }
}
