// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Threading;

#if SILVERLIGHT
using System.Windows.Controls;
#endif

public class AllTests : TestCommon.TestApp
{
    private class Callback
    {
        internal Callback()
        {
            _called = false;
        }

        public void check()
        {
            _m.Lock();
            try
            {
                while(!_called)
                {
                    _m.Wait();
                }

                _called = false;
            }
            finally
            {
                _m.Unlock();
            }
        }

        public void called()
        {
            _m.Lock();
            try
            {
                Debug.Assert(!_called);
                _called = true;
                _m.Notify();
            }
            finally
            {
                _m.Unlock();
            }
        }

        private bool _called;
        private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
    }

    private class AMIRegular
    {
        public void response()
        {
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMIException
    {
        public void response()
        {
            test(false);
        }

        public void exception(Ice.Exception ex)
        {
            test(ex is Ice.ConnectionLostException);
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }


#if SILVERLIGHT
    public override Ice.InitializationData initData()
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties();
        //
        // For this test, we want to disable retries.
        //
        initData.properties.setProperty("Ice.RetryIntervals", "-1");

        //
        // This test kills connections, so we don't want warnings.
        //
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        return initData;
    }

    override
    public void run(Ice.Communicator communicator)
#else
    public static Test.RetryPrx allTests(Ice.Communicator communicator)
#endif
    {
        Write("testing stringToProxy... ");
        Flush();
        string rf = "retry:default -p 12010";
        Ice.ObjectPrx base1 = communicator.stringToProxy(rf);
        test(base1 != null);
        Ice.ObjectPrx base2 = communicator.stringToProxy(rf);
        test(base2 != null);
        WriteLine("ok");

        Write("testing checked cast... ");
        Flush();
        Test.RetryPrx retry1 = Test.RetryPrxHelper.checkedCast(base1);
        test(retry1 != null);
        test(retry1.Equals(base1));
        Test.RetryPrx retry2 = Test.RetryPrxHelper.checkedCast(base2);
        test(retry2 != null);
        test(retry2.Equals(base2));
        WriteLine("ok");

        Write("calling regular operation with first proxy... ");
        Flush();
        retry1.op(false);
        WriteLine("ok");

        Write("calling operation to kill connection with second proxy... ");
        Flush();
        try
        {
            retry2.op(true);
            test(false);
        }
        catch(Ice.ConnectionLostException)
        {
            WriteLine("ok");
        }

        Write("calling regular operation with first proxy again... ");
        Flush();
        retry1.op(false);
        WriteLine("ok");

        AMIRegular cb1 = new AMIRegular();
        AMIException cb2 = new AMIException();

        Write("calling regular AMI operation with first proxy... ");
        retry1.begin_op(false).whenCompleted(cb1.response, cb1.exception);
        cb1.check();
        WriteLine("ok");

        Write("calling AMI operation to kill connection with second proxy... ");
        retry2.begin_op(true).whenCompleted(cb2.response, cb2.exception);
        cb2.check();
        WriteLine("ok");

        Write("calling regular AMI operation with first proxy again... ");
        retry1.begin_op(false).whenCompleted(cb1.response, cb1.exception);
        cb1.check();
        WriteLine("ok");
#if SILVERLIGHT
        retry1.shutdown();
#else
        return retry1;
#endif
    }
}
