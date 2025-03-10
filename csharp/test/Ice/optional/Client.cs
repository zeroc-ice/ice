// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.optional
{
        public class Client : TestHelper
        {
            public override async Task runAsync(string[] args)
            {
                var initData = new InitializationData();
                initData.properties = createTestProperties(ref args);
                using (var communicator = initialize(initData))
                {
                    var initial = await AllTests.allTests(this);
                    initial.shutdown();
                }
            }

            public static Task<int> Main(string[] args) =>
                TestDriver.runTestAsync<Client>(args);
        }
    }

