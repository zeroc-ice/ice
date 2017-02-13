// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Collections.Generic;

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
        public override void catch_async(@abstract.AMD_case_catch cb__, int @checked, Ice.Current current__)
        {
            int @continue = 0;
            cb__.ice_response(@continue);
        }
    }

    public sealed class decimalI : @abstract.decimalDisp_
    {
        public override void @default(Ice.Current current__)
        {
        }
    }

    public sealed class delegateI : @abstract.@delegate
    {
        public override void foo(@abstract.casePrx @else, out int @event, Ice.Current current__)
        {
            @event = 0;
        }
    }

    public sealed class explicitI : @abstract.@explicit
    {
        public override void catch_async(@abstract.AMD_case_catch cb__, int @checked, Ice.Current current__)
        {
            int @continue = 0;
            cb__.ice_response(@continue);
        }

        public override void @default(Ice.Current current)
        {
            test(current.operation == "default");
        }

        public override void foo(@abstract.casePrx @else, out int @event, Ice.Current current__)
        {
            @event = 0;
        }
    }

    public sealed class implicitI : @abstract.@implicit
    {
        public @abstract.@as @in(@abstract.@break @internal, @abstract.@delegate @is, @abstract.@explicit @lock,
                                 @abstract.casePrx @namespace, @abstract.decimalPrx @new, @abstract.delegatePrx @null,
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
        @abstract.@delegate e = new delegateI();
        test(e != null);
        @abstract.@delegatePrx e1 = null;
        test(e1 == null);
        @abstract.@explicit f = new explicitI();
        test(f != null);
        @abstract.@explicitPrx f1 = null;
        if(f1 != null)
        {
            f1.@catch(0, out c2);
            f1.@default();
        }
        test(f1 == null);
        @abstract.@extern l = new @abstract.@extern();
        test(l != null);
        @abstract.@finally g = new @abstract.@finally();
        test(g != null);
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
        communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010:udp");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new decimalI(), communicator.stringToIdentity("test"));
        adapter.add(new Test1I(), communicator.stringToIdentity("test1"));
        adapter.add(new Test2I(), communicator.stringToIdentity("test2"));
        adapter.activate();

        Console.Out.Write("testing operation name... ");
        Console.Out.Flush();
        @abstract.@decimalPrx p = @abstract.@decimalPrxHelper.uncheckedCast(
            adapter.createProxy(communicator.stringToIdentity("test")));
        p.@default();
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing System as module name... ");
        Console.Out.Flush();
        @abstract.System.TestPrx t1 = @abstract.System.TestPrxHelper.uncheckedCast(
                adapter.createProxy(communicator.stringToIdentity("test1")));
        t1.op();

        System.TestPrx t2 = System.TestPrxHelper.uncheckedCast(
                adapter.createProxy(communicator.stringToIdentity("test2")));

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
            try
            {
                communicator.destroy();
            }
            catch(Ice.LocalException ex)
            {
                Console.Error.WriteLine(ex);
                status = 1;
            }
        }

        return status;
    }
}
