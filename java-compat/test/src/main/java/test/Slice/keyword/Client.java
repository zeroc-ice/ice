// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Slice.keyword;
import test.Slice.keyword._abstract.AMD_catch_checkedCast;
import test.Slice.keyword._abstract._assert;
import test.Slice.keyword._abstract._break;
import test.Slice.keyword._abstract._catch;
import test.Slice.keyword._abstract._catchDisp;
import test.Slice.keyword._abstract._default;
import test.Slice.keyword._abstract._defaultDisp;
import test.Slice.keyword._abstract._else;
import test.Slice.keyword._abstract._finalize;
import test.Slice.keyword._abstract._hashCode;
import test.Slice.keyword._abstract._import;
import test.Slice.keyword._abstract._new;
import test.Slice.keyword._abstract._switch;
import test.Slice.keyword._abstract.catchPrx;
import test.Slice.keyword._abstract.defaultPrx;
import test.Slice.keyword._abstract.defaultPrxHelper;
import test.Slice.keyword._abstract.elsePrx;
import test.Slice.keyword._abstract.finalizePrx;
import test.Slice.keyword._abstract.forHolder;
import test.Slice.keyword._abstract.gotoHolder;

public class Client
{
    static public class catchI extends _catchDisp
    {
        public
        catchI()
        {
        }

        @Override
        public void
        checkedCast_async(AMD_catch_checkedCast cb, int _clone, Ice.Current current)
        {
            int _continue = 0;
            cb.ice_response(_continue);
        }
    }

    static public class defaultI extends _defaultDisp
    {
        public
        defaultI()
        {
        }

        @Override
        public void
        _do(Ice.Current current)
        {
            assert current.operation.equals("do");
        }
    }

    static public class elseI extends _else
    {
        public
        elseI()
        {
        }

        @Override
        public void
        foo(defaultPrx _equals, Ice.IntHolder _final, Ice.Current current)
        {
        }
    }

    static public class newI implements _new
    {
        public
        newI()
        {
        }

        @Override
        public _assert
        _notify(_break _notifyAll, _else _null, _finalize _package,
                elsePrx _private, finalizePrx _protected,
                catchPrx _public, defaultPrx _return, int _static, int _strictfp, int _super)
            throws _hashCode, _import
        {
            return null;
        }
    }

    static public class finalizeI extends _finalize
    {
        public
        finalizeI()
        {
        }

        @Override
        public void
        checkedCast_async(AMD_catch_checkedCast cb, int _clone, Ice.Current current)
        {
            int _continue = 0;
            cb.ice_response(_continue);
        }

        @Override
        public void
        _do(Ice.Current current)
        {
        }

        @Override
        public void
        foo(defaultPrx _equals, Ice.IntHolder _final, Ice.Current current)
        {
        }
    }

    //
    // This section of the test is present to ensure that the C++ types
    // are named correctly. It is not expected to run.
    //
    @SuppressWarnings({ "unused", "null" })
    private static void
    testtypes()
    {
        _assert v = _assert._boolean;
        _break b = new _break();
        b._case = 0;
        catchPrx c = null;
        c._checkedCast(0, new Ice.IntHolder());
        _catch c1 = new catchI();
        defaultPrx d = null;
        d._do();
        _default d1 = new defaultI();
        elsePrx e;
        _else e1 = new elseI();
        finalizePrx f = null;
        f._checkedCast(0, new Ice.IntHolder());
        f._do();
        _finalize f1 = new finalizeI();
        forHolder g;
        gotoHolder h;
        _hashCode i = new _hashCode();
        i._if = 0;
        _import j = new _import();
        j._if = 0;
        j._instanceof = 1;
        j._native = 2;
        _new k = new newI();
        assert _switch.value == 0;
    }

    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new defaultI(), Ice.Util.stringToIdentity("test"));
        adapter.activate();

        System.out.print("Testing operation name... ");
        System.out.flush();
        defaultPrx p = defaultPrxHelper.uncheckedCast(
            adapter.createProxy(Ice.Util.stringToIdentity("test")));
        p._do();
        System.out.println("ok");

        return 0;
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            //
            // In this test, we need at least two threads in the
            // client side thread pool for nested AMI.
            //
            Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties(argsH);
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

            communicator = Ice.Util.initialize(argsH, initData);
            status = run(argsH.value, communicator);
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
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
                status = 1;
            }
        }

        System.gc();
        System.exit(status);
    }
}
