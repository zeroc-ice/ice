// Copyright (c) ZeroC, Inc.

using Ice.objects.Test;
using Test;

namespace Ice.objects;

public class Client : TestHelper
{
    public override void run(string[] args)
    {
        var initData = new InitializationData();
        initData.properties = createTestProperties(ref args);
        initData.sliceLoader = new CustomSliceLoader();

        using Communicator communicator = initialize(initData);
        InitialPrx initial = Test.AllTests.allTests(this);
        initial.shutdown();
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
