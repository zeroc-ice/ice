// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.location
{


        public class Client : TestHelper
        {
            public override async Task runAsync(string[] args)
            {
                Ice.Properties properties = createTestProperties(ref args);
                properties.setProperty("Ice.Default.Locator", "locator:" + getTestEndpoint(properties, 0));
                using (var communicator = initialize(properties))
                {
                    await AllTests.allTests(this);
                }
            }

            public static Task<int> Main(string[] args) =>
                TestDriver.runTestAsync<Client>(args);
        }
    }

