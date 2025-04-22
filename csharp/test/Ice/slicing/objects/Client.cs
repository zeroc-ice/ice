// Copyright (c) ZeroC, Inc.

using Test;

public class Client : Test.TestHelper
{
    public override void run(string[] args)
    {
        var initData = new Ice.InitializationData();
        initData.properties = createTestProperties(ref args);
        initData.sliceLoader = new AllTests.CustomSliceLoader();

        using var communicator = initialize(initData);
        TestIntfPrx test = AllTests.allTests(this, false);
        test.shutdown();
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);

}
