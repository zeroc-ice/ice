// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.objects;

public class Client : TestHelper
{
    public override void run(string[] args)
    {
        var initData = new InitializationData();
        initData.properties = createTestProperties(ref args);
        initData.sliceLoader = new CustomSliceLoader();

        using var communicator = initialize(initData);
        var initial = Test.AllTests.allTests(this);
        initial.shutdown();
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);

    private class CustomSliceLoader : SliceLoader
    {
        public object createInstance(string typeId) =>
            typeId switch
            {
                "::Test::B" => new BI(),
                "::Test::C" => new CI(),
                "::Test::D" => new DI(),
                _ => null
            };
    }
}
