// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Threading.Tasks;

public class Client
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
        }
    }

    public sealed class caseI : @abstract.caseDisp_
    {
        public override Task<int>
        catchAsync(int @checked,  Ice.Current current)
        {
            return Task<int>.FromResult(0);
        }
    }

    public sealed class decimalI : @abstract.decimalDisp_
    {
        public override void @default(Ice.Current current)
        {
        }
    }

    public sealed class explicitI : @abstract.explicitDisp_
    {
        public override Task<int>
        catchAsync(int @checked,  Ice.Current current)
        {
            return Task<int>.FromResult(0);
        }

        public override void @default(Ice.Current current)
        {
            test(current.operation == "default");
        }
    }

    public sealed class implicitI : @abstract.@implicit
    {
        public @abstract.@as @in(@abstract.@break @internal, @abstract.@delegate @is, @abstract.@explicit @lock,
                                 @abstract.casePrx @namespace, @abstract.decimalPrx @new, @abstract.@delegate @null,
                                 @abstract.explicitPrx @operator, int @override, int @params, int @private)
        {
            return @abstract.@as.@base;
        }
    }

    public sealed class Test1I : @abstract.System.TestDisp_
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
        if(c1 != null)
        {
            c1.@catch(0, out c2);
        }
        @abstract.@decimal d = new decimalI();
        test(d != null);
        @abstract.@decimalPrx d1 = null;
        if(d1 != null)
        {
            d1.@default();
        }
        test(d1 == null);
        @abstract.@delegate e = new @abstract.@delegate();
        test(e != null);
        @abstract.@delegate e1 = null;
        test(e1 == null);
        @abstract.@explicitPrx f1 = null;
        if(f1 != null)
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
        @abstract.@implicit j = new implicitI();
        test(j != null);
        int k = @abstract.@protected.value;
        test(k == 0);
    }

    private static int run(string[] args, Ice.Communicator communicator)
    {
        communicator.getProperties().setProperty("TestAdapter.Endpoints", "default");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new decimalI(), Ice.Util.stringToIdentity("test"));
        adapter.add(new Test1I(), Ice.Util.stringToIdentity("test1"));
        adapter.add(new Test2I(), Ice.Util.stringToIdentity("test2"));
        adapter.activate();

        Console.Out.Write("testing operation name... ");
        Console.Out.Flush();
        @abstract.@decimalPrx p = @abstract.@decimalPrxHelper.uncheckedCast(
            adapter.createProxy(Ice.Util.stringToIdentity("test")));
        p.@default();
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing System as module name... ");
        Console.Out.Flush();
        @abstract.System.TestPrx t1 = @abstract.System.TestPrxHelper.uncheckedCast(
                adapter.createProxy(Ice.Util.stringToIdentity("test1")));
        t1.op();

        System.TestPrx t2 = System.TestPrxHelper.uncheckedCast(
                adapter.createProxy(Ice.Util.stringToIdentity("test2")));

        t2.op();
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing types... ");
        Console.Out.Flush();
        testtypes();
        Console.Out.WriteLine("ok");

        return 0;
    }

    public static int Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties(ref args);
            communicator = Ice.Util.initialize(ref args, initData);
            status = run(args, communicator);
        }
        catch(System.Exception ex)
        {
            Console.Error.WriteLine(ex);
            status = 1;
        }

        if(communicator != null)
        {
            communicator.destroy();
        }

        return status;
    }
}
