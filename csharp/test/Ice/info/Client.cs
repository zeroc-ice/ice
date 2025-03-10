// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.info
{
        public class Client : TestHelper
        {
            public override void run(string[] args)
            {
                using var communicator = initialize(ref args);
                AllTests.allTests(this);
            }

            public static Task<int> Main(string[] args) =>
                TestDriver.runTestAsync<Client>(args);
        }
    }

