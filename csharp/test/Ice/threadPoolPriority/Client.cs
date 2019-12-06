//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace threadPoolPriority
    {
        public class Client : global::Test.TestHelper
        {
            public override void run(string[] args)
            {
                using (var communicator = initialize(ref args))
                {
                    var output = getWriter();
                    output.Write("testing server priority... ");
                    output.Flush();
                    var priority = Test.PriorityPrx.Parse($"test:{getTestEndpoint(0)} -t 10000", communicator);
                    test("AboveNormal".Equals(priority.getPriority()));
                    output.WriteLine("ok");
                    priority.shutdown();
                }
            }

            public static int Main(string[] args)
            {
                return global::Test.TestDriver.runTest<Client>(args);
            }
        }
    }
}
