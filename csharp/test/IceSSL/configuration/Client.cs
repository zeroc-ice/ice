//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

public class Client : Test.TestHelper
{
    public override void Run(string[] args)
    {
        using var communicator = Initialize(ref args);
        if (args.Length < 1)
        {
            throw new ArgumentException("Usage: client testdir");
        }

        Test.IServerFactoryPrx factory;
        factory = AllTests.allTests(this, args[0]);
        factory.shutdown();
    }

    public static int Main(string[] args) => Test.TestDriver.RunTest<Client>(args);
}
