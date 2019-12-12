//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using Test;
using Ice;

public class Client : TestHelper
{
    private static void allTests(Ice.Communicator communicator)
    {
        Console.Out.Write("testing equals() for Slice structures... ");
        Console.Out.Flush();

        //
        // Define some default values.
        //
        C def_cls = new C(5);
        S1 def_s = new S1("name");
        string[] def_ss = new string[] { "one", "two", "three" };
        int[] def_il = new int[3];
        def_il[0] = 1;
        def_il[1] = 2;
        def_il[2] = 3;
        Dictionary<string, string> def_sd = new Dictionary<string, string>();
        def_sd.Add("abc", "def");
        var def_prx = Ice.IObjectPrx.Parse("test", communicator);
        S2 def_s2 = new S2(true, 98, 99, 100, 101, 1.0f, 2.0, "string", def_ss, def_il, def_sd, def_s, def_cls, def_prx);

        //
        // Compare default-constructed structures.
        //
        {
            test(new S2().Equals(new S2()));
        }

        Console.Out.WriteLine("ok");
    }

    public override void run(string[] args)
    {
        using (var communicator = initialize(ref args))
        {
            allTests(communicator);
        }
    }

    public static int Main(string[] args)
    {
        return TestDriver.runTest<Client>(args);
    }
}
