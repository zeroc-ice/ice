// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.Slice.Test.Escape.@abstract;
using ZeroC.Test;

public class Client : TestHelper
{
    public sealed class Case : Icase
    {
        public ValueTask<int> catchAsync(int @checked, ZeroC.Ice.Current current, CancellationToken cancel) =>
            new(0);
    }

    public sealed class Decimal : Idecimal
    {
        public void @default(ZeroC.Ice.Current current, CancellationToken cancel)
        {
        }
    }

    public sealed class Explicit : Iexplicit
    {
        public ValueTask<int> catchAsync(int @checked, ZeroC.Ice.Current current, CancellationToken cancel) =>
            new(0);

        public void @default(ZeroC.Ice.Current current, CancellationToken cancel) => Assert(current.Operation == "default");
    }

    public sealed class Test1I : ZeroC.Slice.Test.Escape.@abstract.System.ITest
    {
        public void op(ZeroC.Ice.Current current, CancellationToken cancel)
        {
        }
    }

    public sealed class Test2I : ZeroC.Slice.Test.Escape.System.ITest
    {
        public void op(ZeroC.Ice.Current current, CancellationToken cancel)
        {
        }
    }

    public static void TestTypes()
    {
        @as a = @as.@base;
        Assert(a == @as.@base);
        var b = new @break();
        b.@readonly = 0;
        Assert(b.@readonly == 0);
        var c = new Case();
        Assert(c != null);
        IcasePrx? c1 = null;
        Assert(c1 == null);
        int c2;
        if (c1 != null)
        {
            c2 = c1.@catch(0);
            Assert(c2 == 0);
        }
        var d = new Decimal();
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
            Assert(c2 == 0);
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
        int j = Constants.@protected;
        Assert(j == 0);
        int k = Constants.@public;
        Assert(k == 1);
    }

    public override async Task RunAsync(string[] args)
    {
        var endpoint = Protocol == ZeroC.Ice.Protocol.Ice1 ? $"{Transport} -h {Host}" : $"ice+{Transport}://{Host}:0";
        Communicator.SetProperty("TestAdapter.Endpoints", endpoint);

        ZeroC.Ice.ObjectAdapter adapter = Communicator.CreateObjectAdapter("TestAdapter");
        adapter.Add("test", new Decimal());
        adapter.Add("test1", new Test1I());
        adapter.Add("test2", new Test2I());
        await adapter.ActivateAsync();

        Output.Write("testing operation name... ");
        Output.Flush();
        IdecimalPrx p = adapter.CreateProxy("test", IdecimalPrx.Factory);
        p.@default();
        Output.WriteLine("ok");

        Output.Write("testing System as module name... ");
        Output.Flush();
        ZeroC.Slice.Test.Escape.@abstract.System.ITestPrx t1 = adapter.CreateProxy("test1",
            ZeroC.Slice.Test.Escape.@abstract.System.ITestPrx.Factory);
        t1.op();

        ZeroC.Slice.Test.Escape.System.ITestPrx t2 = adapter.CreateProxy("test2",
            ZeroC.Slice.Test.Escape.System.ITestPrx.Factory);
        t2.op();
        Output.WriteLine("ok");

        Output.Write("testing types... ");
        Output.Flush();
        TestTypes();
        Output.WriteLine("ok");
    }

    public static async Task<int> Main(string[] args)
    {
        await using var communicator = CreateCommunicator(ref args);
        await communicator.ActivateAsync();
        return await RunTestAsync<Client>(communicator, args);
    }
}
