// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Client
{
    static public class catchI extends _abstract._catchDisp
    {
        public
        catchI()
        {
        }
        
        public void
        checkedCast_async(_abstract.AMD_catch_checkedCast __cb, int _clone, Ice.Current __current)
        {
            int _continue = 0;
            __cb.ice_response(_continue);
        }
    }

    static public class defaultI extends _abstract._defaultDisp
    {
        public
        defaultI()
        {
        }

        public void
        _do(Ice.Current __current)
        {
            assert __current.operation.equals("do");
        }
    }

    static public class elseI extends _abstract._else
    {
        public
        elseI()
        {
        }

        public void
        foo(_abstract.defaultPrx _equals, Ice.IntHolder _final, Ice.Current __current)
        {
        }
    }

    static public class newI implements _abstract._new
    {
        public
        newI()
        {
        }

        public _abstract._assert
        _notify(_abstract._break _notifyAll, _abstract._else _null, _abstract._finalize _package,
                _abstract.elsePrx _private, _abstract.finalizePrx _protected,
                _abstract.catchPrx _public, _abstract.defaultPrx _return, int _static, int _strictfp, int _super)
            throws _abstract._hashCode, _abstract._import
        {
            return null;
        }
    }

    static public class finalizeI extends _abstract._finalize
    {
        public
        finalizeI()
        {
        }

        public void
        checkedCast_async(_abstract.AMD_catch_checkedCast __cb, int _clone, Ice.Current __current)
        {
            int _continue = 0;
            __cb.ice_response(_continue);
        }

        public void
        _do(Ice.Current __current)
        {
        }

        public void
        foo(_abstract.defaultPrx _equals, Ice.IntHolder _final, Ice.Current __current)
        {
        }
    }

    //
    // This section of the test is present to ensure that the C++ types
    // are named correctly. It is not expected to run.
    //
    private static void
    testtypes()
    {
        _abstract._assert v = _abstract._assert._boolean;
        _abstract._break b = new _abstract._break();
        b._case = 0;
        _abstract.catchPrx c = null;
        c._checkedCast(0, new Ice.IntHolder());
        _abstract._catch c1 = new catchI();
        _abstract.defaultPrx d = null;
        d._do();
        _abstract._default d1 = new defaultI();
        _abstract.elsePrx e;
        _abstract._else e1 = new elseI();
        _abstract.finalizePrx f = null;
        f._checkedCast(0, new Ice.IntHolder());
        f._do();
        _abstract._finalize f1 = new finalizeI();
        _abstract.forHolder g;
        _abstract.gotoHolder h;
        _abstract._hashCode i = new _abstract._hashCode();
        i._if = 0;
        _abstract._import j = new _abstract._import();
        j._if = 0;
        j._instanceof = 1;
        j._native = 2;
        _abstract._new k = new newI();
        assert _abstract._switch.value == 0;
    }

    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010:udp");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new defaultI(), communicator.stringToIdentity("test"));
        adapter.activate();

        System.out.print("Testing operation name... ");
        System.out.flush();
        _abstract.defaultPrx p = _abstract.defaultPrxHelper.uncheckedCast(
            adapter.createProxy(communicator.stringToIdentity("test")));
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
            initData.properties.setProperty("Ice.ThreadPool.Client.Size", "2");
            initData.properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");

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
