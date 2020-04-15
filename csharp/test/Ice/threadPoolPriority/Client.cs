//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice.threadPoolPriority
{
    public class Client : TestHelper
    {
        public override void Run(string[] args)
        {
            using var communicator = Initialize(ref args);
            var output = GetWriter();
            output.Write("testing server priority... ");
            output.Flush();
            var priority = Test.IPriorityPrx.Parse($"test:{GetTestEndpoint(0)} -t 10000", communicator);
            Assert("AboveNormal".Equals(priority.getPriority()));
            output.WriteLine("ok");
            priority.shutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
