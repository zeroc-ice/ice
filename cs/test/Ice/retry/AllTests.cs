// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
            test(ex is Ice.ConnectionLostException || ex is Ice.UnknownLocalException);
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
        initData.observer = Instrumentation.getObserver();

        initData.properties.setProperty("Ice.RetryIntervals", "0 10 20 30");

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

        int invocationCount = 3;

        Write("calling operation to kill connection with second proxy... ");
        Flush();
        try
        {
            retry2.op(true);
            test(false);
        }
        catch(Ice.UnknownLocalException)
        {
            // Expected with collocation
        }
        catch(Ice.ConnectionLostException)
        {
        }
        Instrumentation.testInvocationCount(invocationCount + 1);
        Instrumentation.testFailureCount(1);
        Instrumentation.testRetryCount(0);
        WriteLine("ok");

        Write("calling regular operation with first proxy again... ");
        Flush();
        retry1.op(false);
        Instrumentation.testInvocationCount(invocationCount + 2);
        Instrumentation.testFailureCount(1);
        Instrumentation.testRetryCount(0);
        WriteLine("ok");

        AMIRegular cb1 = new AMIRegular();
        AMIException cb2 = new AMIException();

        Write("calling regular AMI operation with first proxy... ");
        retry1.begin_op(false).whenCompleted(cb1.response, cb1.exception);
        cb1.check();
        Instrumentation.testInvocationCount(invocationCount + 3);
        Instrumentation.testFailureCount(1);
        Instrumentation.testRetryCount(0);
        WriteLine("ok");

        Write("calling AMI operation to kill connection with second proxy... ");
        retry2.begin_op(true).whenCompleted(cb2.response, cb2.exception);
        cb2.check();
        Instrumentation.testInvocationCount(invocationCount + 4);
        Instrumentation.testFailureCount(2);
        Instrumentation.testRetryCount(0);
        WriteLine("ok");

        Write("calling regular AMI operation with first proxy again... ");
        retry1.begin_op(false).whenCompleted(cb1.response, cb1.exception);
        cb1.check();
        Instrumentation.testInvocationCount(invocationCount + 5);
        Instrumentation.testFailureCount(2);
        Instrumentation.testRetryCount(0);
        WriteLine("ok");

        Write("testing idempotent operation... ");
        test(retry1.opIdempotent(0) == 4);
        Instrumentation.testInvocationCount(invocationCount + 6);
        Instrumentation.testFailureCount(2);
        Instrumentation.testRetryCount(4);
        test(retry1.end_opIdempotent(retry1.begin_opIdempotent(4)) == 8);
        Instrumentation.testInvocationCount(invocationCount + 7);
        Instrumentation.testFailureCount(2);
        Instrumentation.testRetryCount(8);
        WriteLine("ok");

        Write("testing non-idempotent operation... ");
        try
        {
            retry1.opNotIdempotent(8);
            test(false);
        }
        catch(Ice.LocalException)
        {
        }
        Instrumentation.testInvocationCount(invocationCount + 8);
        Instrumentation.testFailureCount(3);
        Instrumentation.testRetryCount(8);
        try
        {
            retry1.end_opNotIdempotent(retry1.begin_opNotIdempotent(9));
            test(false);
        }
        catch(Ice.LocalException)
        {
        }
        Instrumentation.testInvocationCount(invocationCount + 9);
        Instrumentation.testFailureCount(4);
        Instrumentation.testRetryCount(8);
        WriteLine("ok");

        if(retry1.ice_getConnection() == null)
        {
            invocationCount = invocationCount + 10;
            Write("testing system exception... ");
            try
            {
                retry1.opSystemException();
                test(false);
            }
            catch(SystemFailure)
            {
            }
            Instrumentation.testInvocationCount(invocationCount + 1);
            Instrumentation.testFailureCount(5);
            Instrumentation.testRetryCount(8);
            try
            {
                retry1.end_opSystemException(retry1.begin_opSystemException());
                test(false);
            }
            catch(SystemFailure)
            {
            }
            Instrumentation.testInvocationCount(invocationCount + 2);
            Instrumentation.testFailureCount(6);
            Instrumentation.testRetryCount(8);
            WriteLine("ok");
        }

#if SILVERLIGHT
        retry1.shutdown();
#else
        return retry1;
#endif
    }
}
