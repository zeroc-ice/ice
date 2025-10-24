// Copyright (c) ZeroC, Inc.

namespace Slice.escape;

public class Client : Test.TestHelper
{
    public sealed class caseI : @abstract.caseDisp_
    {
        public override Task<int>
        catchAsync(int @checked, Ice.Current current) => Task<int>.FromResult(0);
    }

    public sealed class decimalI : @abstract.decimalDisp_
    {
        public override void @default(Ice.Current current)
        {
        }
    }

    public sealed class TotallyDifferentI : @abstract.TotallyDifferentDisp_
    {
        public override Task<int>
        catchAsync(int @checked, Ice.Current current) => Task<int>.FromResult(0);

        public override void @default(Ice.Current current) => test(current.operation == "default");
    }

    public sealed class Test1I : @abstract.System.TestDisp_
    {
        public override void op(Ice.Current current)
        {
        }
    }

    public sealed class Test2I : System.TestDisp_
    {
        public override void op(Ice.Current current)
        {
        }
    }

    private static void
    testtypes()
    {
        @abstract.@as a = @abstract.@as.@base;
        test(a == @abstract.@as.@base);
        var b = new @abstract.@break();
        b.@readonly = 0;
        test(b.@readonly == 0);
        @abstract.@case c = new caseI();
        test(c != null);
        @abstract.@casePrx c1 = null;
        test(c1 == null);
        c1?.@catch(0, out _);
        @abstract.@decimal d = new decimalI();
        test(d != null);
        @abstract.@decimalPrx d1 = null;
        d1?.@default();
        test(d1 == null);
        var e = new @abstract.@delegate();
        test(e != null);
        @abstract.@delegate e1 = null;
        test(e1 == null);
        @abstract.@TotallyDifferentPrx f1 = null;
        if (f1 != null)
        {
            f1.@catch(0, out _);
            f1.@default();
        }
        test(f1 == null);
        var g2 = new Dictionary<string, @abstract.@break>();
        test(g2 != null);
        var h = new @abstract.@fixed();
        h.@for = 0;
        test(h != null);
        var i = new @abstract.@foreach();
        i.@for = 0;
        i.@goto = 1;
        i.@if = 2;
        test(i != null);
        int j = @abstract.@protected.value;
        test(j == 0);
        int k = @abstract.@struct.value;
        test(k == 1);
    }

    public override void run(string[] args)
    {
        using Ice.Communicator communicator = initialize(ref args);
        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new decimalI(), Ice.Util.stringToIdentity("test"));
        adapter.add(new Test1I(), Ice.Util.stringToIdentity("test1"));
        adapter.add(new Test2I(), Ice.Util.stringToIdentity("test2"));
        adapter.activate();

        Console.Out.Write("testing operation name... ");
        Console.Out.Flush();
        @abstract.@decimalPrx p =
            @abstract.@decimalPrxHelper.uncheckedCast(adapter.createProxy(Ice.Util.stringToIdentity("test")));
        p.@default();
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing System as module name... ");
        Console.Out.Flush();
        @abstract.System.TestPrx t1 =
            @abstract.System.TestPrxHelper.uncheckedCast(
                adapter.createProxy(Ice.Util.stringToIdentity("test1")));
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
