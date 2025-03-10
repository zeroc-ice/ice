// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.invoke
{
        public class Client : global::Test.TestHelper
        {
            public override void run(string[] args)
            {
                using (var communicator = initialize(ref args))
                {
                    var myClass = AllTests.allTests(this);
                    myClass.shutdown();
                }
            }

            public static Task<int> Main(string[] args) =>
                TestDriver.runTestAsync<Client>(args);
        }
    }

