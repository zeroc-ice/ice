// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

using System;
using Test;

public class Client : TestHelper
{
    public override void run(string[] args)
    {
        Console.Out.Write("testing Slice predefined macros... ");
        Console.Out.Flush();
        Default d = new Default();
        test(d.x == 10);
        test(d.y == 10);

        NoDefault nd = new NoDefault();
        test(nd.x != 10);
        test(nd.y != 10);

        CsOnly c = new CsOnly();
        test(c.lang.Equals("cs"));
        test(c.version == Ice.Util.intVersion());
        Console.Out.WriteLine("ok");
    }

    public static int Main(string[] args)
    {
        return TestDriver.runTest<Client>(args);
    }
}
