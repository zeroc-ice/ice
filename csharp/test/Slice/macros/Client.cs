// Copyright (c) ZeroC, Inc.

using Test;

public class Client : TestHelper
{
    public override void run(string[] args)
    {
        Console.Out.Write("testing Slice predefined macros... ");
        Console.Out.Flush();
        var d = new Default();
        test(d.x == 10);
        test(d.y == 10);

        var nd = new NoDefault();
        test(nd.x != 10);
        test(nd.y != 10);

        var c = new CsOnly();
        test(c.lang == "cs");
        test(c.version == Ice.Util.intVersion());
        Console.Out.WriteLine("ok");
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
