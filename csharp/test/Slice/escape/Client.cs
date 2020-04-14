//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using @abstract;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

public class Client : TestHelper
{
    public sealed class caseI : Icase
    {
        public ValueTask<int>
        catchAsync(int @checked, Ice.Current current)
        {
            return new ValueTask<int>(0);
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
        public ValueTask<int>
        catchAsync(int @checked, Ice.Current current) => new ValueTask<int>(0);

        public void @default(Ice.Current current) => Assert(current.Operation == "default");
    }

    public sealed class Test1I : @abstract.System.ITest
    {
        public void op(Ice.Current c)
        {
        }
    }

    public sealed class Test2I : ITest
    {
        public void op(Ice.Current c)
        {
        }
    }

    static void
    testtypes()
    {
        var a = @as.@base;
        Assert(a == @as.@base);
        var b = new @break();
        b.@readonly = 0;
        Assert(b.@readonly == 0);
        var c = new caseI();
        Assert(c != null);
        IcasePrx? c1 = null;
        Assert(c1 == null);
        int c2 = 0;
        if (c1 != null)
        {
            c2 = c1.@catch(0);
        }
        var d = new decimalI();
        Assert(d != null);
        IdecimalPrx? d1 = null;
        if (d1 != null)
        {
            d1.@default();
        }
        Assert(d1 == null);
        var e = new @delegate();
        Assert(e != null);
        @delegate? e1 = null;
        Assert(e1 == null);
        IexplicitPrx? f1 = null;
        if (f1 != null)
        {
            c2 = f1.@catch(0);
            f1.@default();
        }
        Assert(f1 == null);
        var g2 = new Dictionary<string, @break>();
        Assert(g2 != null);
        var h = new @fixed();
        h.@for = 0;
        Assert(h != null);
        var i = new @foreach();
        i.@for = 0;
        i.@goto = 1;
        i.@if = 2;
        Assert(i != null);
        var j = @protected.value;
        Assert(j == 0);
    }

    public override void Run(string[] args)
    {
        using (var communicator = Initialize(ref args))
        {
            communicator.SetProperty("TestAdapter.Endpoints", "default");
            Ice.ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("test", new decimalI());
            adapter.Add("test1", new Test1I());
            adapter.Add("test2", new Test2I());
            adapter.Activate();

            Console.Out.Write("testing operation name... ");
            Console.Out.Flush();
            var p = adapter.CreateProxy("test", IdecimalPrx.Factory);
            p.@default();
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing System as module name... ");
            Console.Out.Flush();
            @abstract.System.ITestPrx t1 = adapter.CreateProxy("test1", @abstract.System.ITestPrx.Factory);
            t1.op();

            ITestPrx t2 = adapter.CreateProxy("test2", ITestPrx.Factory);

            t2.op();
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing types... ");
            Console.Out.Flush();
            testtypes();
            Console.Out.WriteLine("ok");
        }
    }

    public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
}
