// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.invoke;

public class Client : global::Test.TestHelper
{
    public override void run(string[] args)
    {
        using Communicator communicator = initialize(ref args);
        Test.MyInterfacePrx myInterface = AllTests.allTests(this);
        myInterface.shutdown();
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
