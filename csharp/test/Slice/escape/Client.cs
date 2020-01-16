//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using @abstract;
using @abstract.System;

public class Client : Test.TestHelper
{
    public sealed class caseI : Icase
    {
        public Task<int>
        catchAsync(int @checked, Ice.Current current)
        {
            return Task.FromResult(0);
        }
    }

    public sealed class decimalI : Idecimal
    {
        public void @default(Ice.Current current)
        {
        }
    }

    public sealed class explicitI : Iexplicit
    {
        public Task<int>
        catchAsync(int @checked, Ice.Current current) => Task.FromResult(0);

        public void @default(Ice.Current current) => test(current.Operation == "default");
    }

    public sealed class Test1I : @abstract.System.ITest
    {
        public void op(Ice.Current c)
        {
        }
    }

    public sealed class Test2I : System.ITest
    {
        public void op(Ice.Current c)
        {
        }
    }

    static void
    testtypes()
    {
        var a = @as.@base;
        test(a == @as.@base);
        var b = new @break();
        b.@readonly = 0;
        test(b.@readonly == 0);
        var c = new caseI();
        test(c != null);
        IcasePrx c1 = null;
        test(c1 == null);
        int c2 = 0;
        if (c1 != null)
        {
            c2 = c1.@catch(0);
        }
        var d = new decimalI();
        test(d != null);
        IdecimalPrx d1 = null;
        if (d1 != null)
        {
            d1.@default();
        }
        test(d1 == null);
        var e = new @delegate();
        test(e != null);
        @delegate e1 = null;
        test(e1 == null);
        IexplicitPrx f1 = null;
        if (f1 != null)
        {
            c2 = f1.@catch(0);
            f1.@default();
        }
        test(f1 == null);
        var g2 = new Dictionary<string, @break>();
        test(g2 != null);
        var h = new @fixed();
        h.@for = 0;
        test(h != null);
        var i = new @foreach();
        i.@for = 0;
        i.@goto = 1;
        i.@if = 2;
        test(i != null);
        var j = @protected.value;
        test(j == 0);
    }

    public override void run(string[] args)
    {
        using (var communicator = initialize(ref args))
        {
            communicator.SetProperty("TestAdapter.Endpoints", "default");
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.Add(new decimalI(), "test");
            adapter.Add(new Test1I(), "test1");
            adapter.Add(new Test2I(), "test2");
            adapter.Activate();

            Console.Out.Write("testing operation name... ");
            Console.Out.Flush();
            var p = IdecimalPrx.UncheckedCast(adapter.CreateProxy("test"));
            p.@default();
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing System as module name... ");
            Console.Out.Flush();
            @abstract.System.ITestPrx t1 = @abstract.System.ITestPrx.UncheckedCast(adapter.CreateProxy("test1"));
            t1.op();

            System.ITestPrx t2 = System.ITestPrx.UncheckedCast(adapter.CreateProxy("test2"));

            t2.op();
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing types... ");
            Console.Out.Flush();
            testtypes();
            Console.Out.WriteLine("ok");
        }
    }

    public static int Main(string[] args) => Test.TestDriver.runTest<Client>(args);
}
