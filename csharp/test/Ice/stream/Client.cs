// Copyright (c) ZeroC, Inc.

namespace Ice.stream;

public class Client : global::Test.TestHelper
{
    public override void run(string[] args)
    {
        var initData = new InitializationData();
        initData.properties = createTestProperties(ref args);
        var customSliceLoader = new AllTests.CustomSliceLoader();
        initData.sliceLoader = customSliceLoader;

        using var communicator = initialize(initData);
        AllTests.allTests(this, customSliceLoader);
    }

    public static Task<int> Main(string[] args) =>
        global::Test.TestDriver.runTestAsync<Client>(args);
}
