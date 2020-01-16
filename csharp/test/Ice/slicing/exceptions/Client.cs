//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

public class Client : TestHelper
{
    public override void run(string[] args)
    {
        using var communicator = initialize(ref args);
        ITestIntfPrx test = AllTests.allTests(this, false);
        test.shutdown();
    }

    public static int Main(string[] args) => TestDriver.runTest<Client>(args);
}
