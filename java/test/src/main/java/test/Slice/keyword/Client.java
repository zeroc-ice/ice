// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Slice.keyword;
import test.Slice.keyword._abstract._assert;
import test.Slice.keyword._abstract._break;
import test.Slice.keyword._abstract._catch;
import test.Slice.keyword._abstract._default;
import test.Slice.keyword._abstract._else;
import test.Slice.keyword._abstract._elseDisp;
import test.Slice.keyword._abstract._finalize;
import test.Slice.keyword._abstract._finalizeDisp;
import test.Slice.keyword._abstract._hashCode;
import test.Slice.keyword._abstract._import;
import test.Slice.keyword._abstract._new;
import test.Slice.keyword._abstract._switch;
import test.Slice.keyword._abstract.catchPrx;
import test.Slice.keyword._abstract.defaultPrx;
import test.Slice.keyword._abstract.elsePrx;
import test.Slice.keyword._abstract.finalizePrx;

public class Client
{
    static public class catchI implements _catch
    {
        public catchI()
        {
        }

        @Override
        public java.util.concurrent.CompletionStage<Integer> checkedCastAsync(int _clone, com.zeroc.Ice.Current current)
        {
            int _continue = 0;
            return java.util.concurrent.CompletableFuture.completedFuture(_continue);
        }
    }

    static public class defaultI implements _default
    {
        public defaultI()
        {
        }

        @Override
        public void _do(com.zeroc.Ice.Current current)
        {
            assert current.operation.equals("do");
        }
    }

    static public class elseI extends _else
    {
        public elseI()
        {
        }
    }

    static public class elseServantI implements _elseDisp
    {
        @Override
        public int foo(defaultPrx _equals, com.zeroc.Ice.Current current)
        {
            return 0;
        }
    }

    static public class newI implements _new
    {
        public newI()
        {
        }

        @Override
        public _assert _notify(_break _notifyAll, _else _null, _finalize _package,
                               elsePrx _private, finalizePrx _protected,
                               catchPrx _public, defaultPrx _return, int _static, int _strictfp, int _super)
            throws _hashCode, _import
        {
            return null;
        }
    }

    static public class finalizeI extends _finalize
    {
        public finalizeI()
        {
        }
    }

    static public class finalizeServantI implements _finalizeDisp
    {
        @Override
        public java.util.concurrent.CompletionStage<Integer> checkedCastAsync(int _clone, com.zeroc.Ice.Current current)
        {
            int _continue = 0;
            return java.util.concurrent.CompletableFuture.completedFuture(_continue);
        }

        @Override
        public void _do(com.zeroc.Ice.Current current)
        {
        }

        @Override
        public int foo(defaultPrx _equals, com.zeroc.Ice.Current current)
        {
            return 0;
        }
    }

    //
    // This section of the test is present to ensure that the C++ types
    // are named correctly. It is not expected to run.
    //
    @SuppressWarnings({ "unused", "null" })
    private static void testtypes()
    {
        _assert v = _assert._boolean;
        _break b = new _break();
        b._case = 0;
        catchPrx c = null;
        c._checkedCast(0);
        _catch c1 = new catchI();
        defaultPrx d = null;
        d._do();
        _default d1 = new defaultI();
        elsePrx e;
        _else e1 = new elseI();
        finalizePrx f = null;
        f._checkedCast(0);
        f._do();
        _finalize f1 = new finalizeI();
        _hashCode i = new _hashCode();
        i._if = 0;
        _import j = new _import();
        j._if = 0;
        j._instanceof = 1;
        j._native = 2;
        _new k = new newI();
        assert _switch.value == 0;
    }

    private static int run(String[] args, com.zeroc.Ice.Communicator communicator)
    {
        com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new defaultI(), com.zeroc.Ice.Util.stringToIdentity("test"));
        adapter.activate();

        System.out.print("Testing operation name... ");
        System.out.flush();
        defaultPrx p = defaultPrx.uncheckedCast(adapter.createProxy(com.zeroc.Ice.Util.stringToIdentity("test")));
        p._do();
        System.out.println("ok");

        return 0;
    }

    public static void main(String[] args)
    {
        int status = 0;
        com.zeroc.Ice.Communicator communicator = null;

        try
        {
            //
            // In this test, we need at least two threads in the
            // client side thread pool for nested AMI.
            //
            com.zeroc.Ice.Util.CreatePropertiesResult cpr = com.zeroc.Ice.Util.createProperties(args);
            com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
            initData.properties = cpr.properties;
            initData.properties.setProperty("Ice.Package._abstract", "test.Slice.keyword");
            initData.properties.setProperty("Ice.ThreadPool.Client.Size", "2");
            initData.properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
            initData.properties.setProperty("TestAdapter.Endpoints", "default");

            //
            // We must set MessageSizeMax to an explicit values,
            // because we run tests to check whether
            // Ice.MemoryLimitException is raised as expected.
            //
            initData.properties.setProperty("Ice.MessageSizeMax", "100");

            com.zeroc.Ice.Util.InitializeResult ir = com.zeroc.Ice.Util.initialize(cpr.args, initData);
            communicator = ir.communicator;
            status = run(ir.args, communicator);
        }
        catch(Exception ex)
        {
            ex.printStackTrace();
            status = 1;
        }

        if(communicator != null)
        {
            try
            {
                communicator.destroy();
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
                ex.printStackTrace();
                status = 1;
            }
        }

        System.gc();
        System.exit(status);
    }
}
