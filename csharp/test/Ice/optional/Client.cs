//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.Optional
{
    public class Client : TestHelper
    {
        public override void Run(string[] args)
        {
            _ = Initialize(ref args);
            AllTests.Run(this).Shutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
