// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.adapterDeactivation
{
        public class Client : TestHelper
        {
            public override async Task runAsync(string[] args)
            {
                using var communicator = initialize(ref args);
                await AllTests.allTests(this);
            }

            public static Task<int> Main(string[] args) =>
                TestDriver.runTestAsync<Client>(args);
        }
    }

