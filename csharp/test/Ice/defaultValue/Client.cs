//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.DefaultValue
{
    public class Client : TestHelper
    {
        public override void Run(string[] args) => AllTests.allTests(this);

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
