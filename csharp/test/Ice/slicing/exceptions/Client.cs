//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

public class Client : TestHelper
{
    public override void Run(string[] args)
    {
        using var communicator = Initialize(ref args);
        ITestIntfPrx test = AllTests.allTests(this, false);
        test.shutdown();
    }

    public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
}
