// Copyright (c) ZeroC, Inc.

public class Client : Test.TestHelper
{
    public sealed class caseI : escaped_abstract.caseDisp_
    {
        public override Task<int>
        catchAsync(int @checked, Ice.Current current) => Task<int>.FromResult(0);
    }

    public sealed class decimalI : escaped_abstract.decimalDisp_
    {
        public override void @default(Ice.Current current)
        {
        }
    }

    public sealed class TotallyDifferentI : escaped_abstract.TotallyDifferentDisp_
    {
        public override Task<int>
        catchAsync(int @checked, Ice.Current current) => Task<int>.FromResult(0);

        public override void @default(Ice.Current current) => test(current.operation == "default");
    }

    public sealed class Test1I : escaped_abstract.System.TestDisp_
    {
        public override void op(Ice.Current c)
        {
        }
    }

    public sealed class Test2I : System.TestDisp_
    {
        public override void op(Ice.Current c)
        {
        }
    }

    private static void
    testtypes()
    {
        escaped_abstract.@as a = escaped_abstract.@as.@base;
        test(a == escaped_abstract.@as.@base);
        var b = new escaped_abstract.@break();
        b.@readonly = 0;
        test(b.@readonly == 0);
        escaped_abstract.@case c = new caseI();
        test(c != null);
        escaped_abstract.@casePrx c1 = null;
        test(c1 == null);
        c1?.@catch(0, out _);
        escaped_abstract.@decimal d = new decimalI();
        test(d != null);
        escaped_abstract.@decimalPrx d1 = null;
        d1?.@default();
        test(d1 == null);
        var e = new escaped_abstract.@delegate();
        test(e != null);
        escaped_abstract.@delegate e1 = null;
        test(e1 == null);
        escaped_abstract.@TotallyDifferentPrx f1 = null;
        if (f1 != null)
        {
            f1.@catch(0, out _);
            f1.@default();
        }
        test(f1 == null);
        var g2 = new Dictionary<string, escaped_abstract.@break>();
        test(g2 != null);
        var h = new escaped_abstract.@fixed();
        h.@for = 0;
        test(h != null);
        var i = new escaped_abstract.@foreach();
        i.@for = 0;
        i.@goto = 1;
        i.@if = 2;
        test(i != null);
        int j = escaped_abstract.@protected.value;
        test(j == 0);
        int k = escaped_abstract.@struct.value;
        test(k == 1);
    }

    public override void run(string[] args)
    {
        using var communicator = initialize(ref args);
        communicator.getProperties().setProperty("TestAdapter.Endpoints", "default");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new decimalI(), Ice.Util.stringToIdentity("test"));
        adapter.add(new Test1I(), Ice.Util.stringToIdentity("test1"));
        adapter.add(new Test2I(), Ice.Util.stringToIdentity("test2"));
        adapter.activate();

        Console.Out.Write("testing operation name... ");
        Console.Out.Flush();
        escaped_abstract.@decimalPrx p =
            escaped_abstract.@decimalPrxHelper.uncheckedCast(adapter.createProxy(Ice.Util.stringToIdentity("test")));
        p.@default();
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing System as module name... ");
        Console.Out.Flush();
        escaped_abstract.System.TestPrx t1 =
            escaped_abstract.System.TestPrxHelper.uncheckedCast(adapter.createProxy(Ice.Util.stringToIdentity("test1")));
        t1.op();

        System.TestPrx t2 =
            System.TestPrxHelper.uncheckedCast(adapter.createProxy(Ice.Util.stringToIdentity("test2")));

        t2.op();
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing types... ");
        Console.Out.Flush();
        testtypes();
        Console.Out.WriteLine("ok");
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Client>(args);
}
