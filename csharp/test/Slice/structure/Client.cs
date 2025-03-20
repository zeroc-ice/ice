// Copyright (c) ZeroC, Inc.

using Test;

public class Client : TestHelper
{
    private static void allTests(Ice.Communicator communicator)
    {
        Console.Out.Write("testing operator== for Slice structures... ");
        Console.Out.Flush();

        //
        // Define some default values.
        //
        var def_cls = new C(5);
        var def_s = new S1("name");
        string[] def_ss = new string[] { "one", "two", "three" };
        var def_il = new List<int> { 1, 2, 3 };
        var def_sd = new Dictionary<string, string>
        {
            { "abc", "def" }
        };
        Ice.ObjectPrx def_prx = communicator.stringToProxy("test");
        var def_s2 = new S2(true, (byte)98, (short)99, 100, 101, (float)1.0, 2.0, "string", def_ss, def_il, def_sd,
                           def_s, def_cls, def_prx);

        //
        // Change one primitive member at a time.
        //
        {
            S2 v;

            v = def_s2 with { };
            test(v == def_s2);

            v = def_s2 with { bo = false };
            test(v != def_s2);

            v = def_s2 with { };
            v.by--;
            test(v != def_s2);

            v = def_s2 with { };
            v.sh--;
            test(v != def_s2);

            v = def_s2 with { };
            v.i--;
            test(v != def_s2);

            v = def_s2 with { };
            v.l--;
            test(v != def_s2);

            v = def_s2 with { };
            v.f--;
            test(v != def_s2);

            v = def_s2 with { };
            v.d--;
            test(v != def_s2);

            v = def_s2 with { str = "" };
            test(v != def_s2);
        }

        //
        // String member
        //
        {
            S2 v1, v2;

            v1 = def_s2 with { str = (string)def_s2.str.Clone() };
            test(v1 == def_s2);

            v1 = def_s2 with { str = "" };
            v2 = def_s2 with { };
            test(v1 != v2);

            v1 = def_s2 with { };
            v2 = def_s2 with { str = "" };
            test(v1 != v2);

            v1 = def_s2 with { str = "" };
            v2 = def_s2 with { str = "" };
            test(v1 == v2);
        }

        //
        // Sequence member
        //
        {
            S2 v1, v2;

            v1 = def_s2 with { ss = (string[])def_s2.ss.Clone() };
            test(v1 != def_s2);

            v1 = def_s2 with { ss = Array.Empty<string>() };
            v2 = def_s2 with { };
            test(v1 != v2);

            v1 = def_s2 with { };
            v2 = def_s2 with { ss = Array.Empty<string>() };
            test(v1 != v2);
        }

        //
        // Custom sequence member
        //
        {
            S2 v1, v2;

            v1 = def_s2 with { il = new List<int>(def_s2.il) };
            test(v1 != def_s2);

            v1 = def_s2 with { il = new List<int> { 0, 0, 0 } };
            test(v1 != def_s2);

            v1 = def_s2 with { il = new List<int>() };
            v2 = def_s2 with { };
            test(v1 != v2);

            v1 = def_s2 with { };
            v2 = def_s2 with { il = new List<int>() };
            test(v1 != v2);
        }

        //
        // Dictionary member
        //
        {
            S2 v1;

            v1 = def_s2 with { sd = new Dictionary<string, string>(def_s2.sd) };
            test(v1 != def_s2);

            v1 = def_s2 with { sd = new Dictionary<string, string>() };
            test(v1 != def_s2);
        }

        //
        // Struct member
        //
        {
            S2 v1;

            v1 = def_s2 with { s = def_s2.s with { } };
            test(v1 == def_s2);

            v1 = def_s2 with { s = new S1("name") };
            test(v1 == def_s2);

            v1 = def_s2 with { s = new S1("noname") };
            test(v1 != def_s2);
        }

        //
        // Class member
        //
        {
            S2 v1, v2;

            v1 = def_s2 with { cls = (C)def_s2.cls.Clone() };
            test(v1 != def_s2);

            v1 = def_s2 with { cls = null };
            v2 = def_s2 with { };
            test(v1 != v2);

            v1 = def_s2 with { };
            v2 = def_s2 with { cls = null };
            test(v1 != v2);
        }

        //
        // Proxy member
        //
        {
            S2 v1, v2;

            v1 = def_s2 with { prx = communicator.stringToProxy("test") };
            test(v1 == def_s2);

            v1 = def_s2 with { prx = communicator.stringToProxy("test2") };
            test(v1 != def_s2);

            v1 = def_s2 with { prx = null };
            v2 = def_s2 with { };
            test(v1 != v2);

            v1 = def_s2 with { };
            v2 = def_s2 with { prx = null };
            test(v1 != v2);
        }

        Console.Out.WriteLine("ok");
    }

    public override void run(string[] args)
    {
        using var communicator = initialize(ref args);
        allTests(communicator);
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
