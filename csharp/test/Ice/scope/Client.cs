// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.scope
{
        public class Client : TestHelper
        {
            public override void run(string[] args)
            {
                var initData = new InitializationData();
                initData.properties = createTestProperties(ref args);
                using (var communicator = initialize(initData))
                {
                    var output = getWriter();
                    output.Write("test using same type name in different Slice modules... ");
                    output.Flush();
                    AllTests.allTests(this);
                    output.WriteLine("ok");
                }
            }

            public static Task<int> Main(string[] args) =>
                TestDriver.runTestAsync<Client>(args);
        }
    }

