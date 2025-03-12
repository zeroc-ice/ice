// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.inheritance
{
    public class Client : TestHelper
    {
        public override void run(string[] args)
        {
            using (var communicator = initialize(ref args))
            {
                var initial = AllTests.allTests(this);
                initial.shutdown();
            }
        }

        public static Task<int> Main(string[] args) =>
            TestDriver.runTestAsync<Client>(args);
    }
}
