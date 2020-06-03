//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using Test;

namespace ZeroC.Ice.Test.Threading
{
    public class Client : TestHelper
    {
        public override void Run(string[] args)
        {
            using var communicator = Initialize(ref args);
            try
            {
                AllTests.allTests(this, false).Result.shutdown();
            }
            catch (AggregateException ex)
            {
                if (ex.InnerException is TestFailedException failedEx)
                {
                    GetWriter().WriteLine($"test failed: {failedEx.reason}");
                    Assert(false);
                }
                throw;
            }
            catch (TestFailedException ex)
            {
                GetWriter().WriteLine($"test failed: {ex.reason}");
                Assert(false);
            }
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
