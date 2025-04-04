// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.dictMapping;

public class Client : TestHelper
{
    public override async Task runAsync(string[] args)
    {
        await using var communicator = initialize(ref args);
        var output = getWriter();
        var myClass = await AllTests.allTests(this, false);
        output.Write("shutting down server... ");
        output.Flush();
        myClass.shutdown();
        output.WriteLine("ok");
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
