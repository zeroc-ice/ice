//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Slice.escape;
import test.Slice.escape._abstract._assert;
import test.Slice.escape._abstract._break;
import test.Slice.escape._abstract._catch;
import test.Slice.escape._abstract._default;
import test.Slice.escape._abstract._else;
import test.Slice.escape._abstract._finalize;
import test.Slice.escape._abstract._hashCode;
import test.Slice.escape._abstract._import;
import test.Slice.escape._abstract._new;
import test.Slice.escape._abstract._switch;
import test.Slice.escape._abstract.catchPrx;
import test.Slice.escape._abstract.defaultPrx;
import test.Slice.escape._abstract.finalizePrx;

public class Client extends test.TestHelper
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

    static public class newI implements _new
    {
        public newI()
        {
        }

        @Override
        public _assert _notify(_break _notifyAll, _else _null, _finalize _package,
                               com.zeroc.Ice.ObjectPrx _private, finalizePrx _protected,
                               catchPrx _public, defaultPrx _return, int _static, int _strictfp, int _super)
            throws _hashCode, _import
        {
            return null;
        }
    }

    static public class finalizeServantI implements _finalize
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
        _else e1 = new elseI();
        e1._if = 0;
        e1._equals = null;
        e1._final = 0;

        finalizePrx f = null;
        f._checkedCast(0);
        f._do();

        _hashCode i = new _hashCode();
        i._if = 0;
        _import j = new _import();
        j._if = 0;
        j._instanceof = 1;
        j._native = 2;
        _new k = new newI();
        assert _switch.value == 0;
    }

    public void run(String[] args)
    {
        //
        // In this test, we need at least two threads in the
        // client side thread pool for nested AMI.
        //
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package._abstract", "test.Slice.escape");
        properties.setProperty("Ice.ThreadPool.Client.Size", "2");
        properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
        properties.setProperty("TestAdapter.Endpoints", "default");

        //
        // We must set MessageSizeMax to an explicit values,
        // because we run tests to check whether
        // Ice.MemoryLimitException is raised as expected.
        //
        properties.setProperty("Ice.MessageSizeMax", "100");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new defaultI(), com.zeroc.Ice.Util.stringToIdentity("test"));
            adapter.activate();

            System.out.print("Testing operation name... ");
            System.out.flush();
            defaultPrx p = defaultPrx.uncheckedCast(adapter.createProxy(com.zeroc.Ice.Util.stringToIdentity("test")));
            p._do();
            System.out.println("ok");
        }
    }
}
