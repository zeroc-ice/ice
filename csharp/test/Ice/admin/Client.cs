//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice
{
    namespace admin
    {
        public class Client : TestHelper
        {
            public override void run(string[] args)
            {
                try
                {
                    using var communicator = initialize(ref args);
                    AllTests.allTests(this);
                }
                catch(System.Exception ex)
                {
                    System.Console.WriteLine(ex.ToString());
                }
            }

            public static int Main(string[] args) => TestDriver.runTest<Client>(args);
        }
    }
}
