// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice
{
    namespace facets
    {
        public class Client : TestHelper
        {
            public override async Task runAsync(string[] args)
            {
                using var communicator = initialize(ref args);
                var g = await AllTests.allTests(this);
                g.shutdown();
            }

            public static Task<int> Main(string[] args) =>
                TestDriver.runTestAsync<Client>(args);
        }
    }
}
