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
            public override void Run(string[] args)
            {
                try
                {
                    using Communicator communicator = Initialize(ref args);
                    AllTests.allTests(this);
                }
                catch (System.Exception ex)
                {
                    System.Console.WriteLine(ex.ToString());
                }
            }

            public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
        }
    }
}
