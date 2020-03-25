//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

public class Client : TestHelper
{
    public override void run(string[] args)
    {
        using var communicator = initialize(ref args);
        AllTests.allTests(this, false).shutdown();
    }

    public static int Main(string[] args) => TestDriver.runTest<Client>(args);
}
