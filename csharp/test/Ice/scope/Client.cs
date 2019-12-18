//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice
{
    namespace scope
    {
        public class Client : TestHelper
        {
            public override void run(string[] args)
            {
                using var communicator = initialize(createTestProperties(ref args),
                    typeIdNamespaces: new string[] { "Ice.scope.TypeId" });
                var output = getWriter();
                output.Write("test using same type name in different Slice modules... ");
                output.Flush();
                AllTests.allTests(this);
                output.WriteLine("ok");
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Client>(args);
            }
        }
    }
}
