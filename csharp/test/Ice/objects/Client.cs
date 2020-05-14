//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.objects
{
    public class Client : TestHelper
    {
        public override void Run(string[] args)
        {
            using var communicator = Initialize(CreateTestProperties(ref args),
                                                typeIdNamespaces: new string[] { "ZeroC.Ice.objects.TypeId" });
            var initial = Test.AllTests.allTests(this);
            initial.shutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
