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
    public sealed class caseI : @abstract.@case
    {
        public Task<int>
        catchAsync(int @checked, Ice.Current current)
        {
            return Task<int>.FromResult(0);
        }
    }

    public sealed class decimalI : @abstract.@decimal
    {
        public void @default(Ice.Current current)
        {
        }
    }

    public sealed class explicitI : @abstract.@explicit
    {
        public Task<int>
        catchAsync(int @checked, Ice.Current current)
        {
            return Task<int>.FromResult(0);
        }

        public void @default(Ice.Current current)
        {
            test(current.Operation == "default");
        }
    }

    public sealed class Test1I : @abstract.System.Test
    {
        public void op(Ice.Current c)
        {
        }
    }

    public sealed class Test2I : System.Test
    {
        public void op(Ice.Current c)
        {
        }
    }

    static void
    testtypes()
    {
        @abstract.@as a = @abstract.@as.@base;
        test(a == @abstract.@as.@base);
        @abstract.@break b = new @abstract.@break();
        b.@readonly = 0;
        test(b.@readonly == 0);
        @abstract.@case c = new caseI();
        test(c != null);
        @abstract.@casePrx c1 = null;
        test(c1 == null);
        int c2 = 0;
        if (c1 != null)
        {
            c1.@catch(0, out c2);
        }
        @abstract.@decimal d = new decimalI();
        test(d != null);
        @abstract.@decimalPrx d1 = null;
        if (d1 != null)
        {
            d1.@default();
        }
        test(d1 == null);
        @abstract.@delegate e = new @abstract.@delegate();
        test(e != null);
        @abstract.@delegate e1 = null;
        test(e1 == null);
        @abstract.@explicitPrx f1 = null;
        if (f1 != null)
        {
            f1.@catch(0, out c2);
            f1.@default();
        }
        test(f1 == null);
        Dictionary<string, @abstract.@break> g2 = new Dictionary<string, @abstract.@break>();
        test(g2 != null);
        @abstract.@fixed h = new @abstract.@fixed();
        h.@for = 0;
        test(h != null);
        @abstract.@foreach i = new @abstract.@foreach();
        i.@for = 0;
        i.@goto = 1;
        i.@if = 2;
        test(i != null);
        int j = @abstract.@protected.value;
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
            @abstract.@decimalPrx p = @abstract.@decimalPrx.UncheckedCast(adapter.CreateProxy("test"));
            p.@default();
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing System as module name... ");
            Console.Out.Flush();
            @abstract.System.TestPrx t1 = @abstract.System.TestPrx.UncheckedCast(adapter.CreateProxy("test1"));
            t1.op();

            System.TestPrx t2 = System.TestPrx.UncheckedCast(adapter.CreateProxy("test2"));

            t2.op();
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing types... ");
            Console.Out.Flush();
            testtypes();
            Console.Out.WriteLine("ok");
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Client>(args);
    }
}
