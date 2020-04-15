//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice.scope
{
    public class Client : TestHelper
    {
        public override void Run(string[] args)
        {
            using var communicator = Initialize(CreateTestProperties(ref args),
                typeIdNamespaces: new string[] { "Ice.scope.TypeId" });
            var output = GetWriter();
            output.Write("test using same type name in different Slice modules... ");
            output.Flush();
            AllTests.allTests(this);
            output.WriteLine("ok");
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
