//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

public class Client : TestHelper
{
    public override void run(string[] args)
    {
        var properties = createTestProperties(ref args);
        properties["Ice.Default.SlicedFormat"] = "1";
        using var communicator = initialize(properties);
        AllTests.allTests(this, false).shutdown();
    }

    public static int Main(string[] args) => TestDriver.runTest<Client>(args);
}
