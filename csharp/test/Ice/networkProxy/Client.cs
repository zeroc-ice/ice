//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

public class Client : Test.TestHelper
{
    public override void Run(string[] args)
    {
        using var communicator = Initialize(ref args);
        AllTests.allTests(this);
    }

    public static int Main(string[] args) => Test.TestDriver.RunTest<Client>(args);
}
