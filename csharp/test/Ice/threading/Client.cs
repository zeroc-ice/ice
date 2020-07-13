//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Threading
{
    public class Client : TestHelper
    {
        public override Task Run(string[] args)
        {
            using var communicator = Initialize(ref args);
            try
            {
                AllTests.allTests(this, false).Result.shutdown();
                return Task.CompletedTask;
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
                throw;
            }
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
