// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.retry;

import java.io.PrintWriter;

import test.Ice.retry.Test.Callback_Retry_op;
import test.Ice.retry.Test.RetryPrx;
import test.Ice.retry.Test.RetryPrxHelper;

public class AllTests
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private static class Callback
    {
        Callback()
        {
            _called = false;
        }

        public synchronized void
        check()
        {
            while(!_called)
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                }
            }

            _called = false;
        }

        public synchronized void
        called()
        {
            assert(!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    private static class AMIRegular extends Callback_Retry_op
    {
        @Override
        public void
        response()
        {
            callback.called();
        }

        @Override
        public void
        exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMIException extends Callback_Retry_op
    {
        @Override
        public void
        response()
        {
            test(false);
        }

        @Override
        public void
        exception(Ice.LocalException ex)
        {
            test(ex instanceof Ice.ConnectionLostException || ex instanceof Ice.UnknownLocalException);
            callback.called();
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    public static RetryPrx
    allTests(Ice.Communicator communicator, PrintWriter out)
    {
        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "retry:default -p 12010";
        Ice.ObjectPrx base1 = communicator.stringToProxy(ref);
        test(base1 != null);
        Ice.ObjectPrx base2 = communicator.stringToProxy(ref);
        test(base2 != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        RetryPrx retry1 = RetryPrxHelper.checkedCast(base1);
        test(retry1 != null);
        test(retry1.equals(base1));
        RetryPrx retry2 = RetryPrxHelper.checkedCast(base2);
        test(retry2 != null);
        test(retry2.equals(base2));
        out.println("ok");

        out.print("calling regular operation with first proxy... ");
        out.flush();
        retry1.op(false);
        out.println("ok");

        int invocationCount = 3;

        out.print("calling operation to kill connection with second proxy... ");
        out.flush();
        try
        {
            retry2.op(true);
            test(false);
        }
        catch(Ice.UnknownLocalException ex)
        {
            // Expected with collocation
        }
        catch(Ice.ConnectionLostException ex)
        {
        }
        Instrumentation.testInvocationCount(invocationCount + 1);
        Instrumentation.testFailureCount(1);
        Instrumentation.testRetryCount(0);
        out.println("ok");
        
        out.print("calling regular operation with first proxy again... ");
        out.flush();
        retry1.op(false);
        Instrumentation.testInvocationCount(invocationCount + 2);
        Instrumentation.testFailureCount(1);
        Instrumentation.testRetryCount(0);
        out.println("ok");

        AMIRegular cb1 = new AMIRegular();
        AMIException cb2 = new AMIException();

        out.print("calling regular AMI operation with first proxy... ");
        retry1.begin_op(false, cb1);
        cb1.check();
        Instrumentation.testInvocationCount(invocationCount + 3);
        Instrumentation.testFailureCount(1);
        Instrumentation.testRetryCount(0);
        out.println("ok");

        out.print("calling AMI operation to kill connection with second proxy... ");
        retry2.begin_op(true, cb2);
        cb2.check();
        Instrumentation.testInvocationCount(invocationCount + 4);
        Instrumentation.testFailureCount(2);
        Instrumentation.testRetryCount(0);
        out.println("ok");

        out.print("calling regular AMI operation with first proxy again... ");
        retry1.begin_op(false, cb1);
        cb1.check();
        Instrumentation.testInvocationCount(invocationCount + 5);
        Instrumentation.testFailureCount(2);
        Instrumentation.testRetryCount(0);
        out.println("ok");

        out.print("testing idempotent operation... ");
        test(retry1.opIdempotent(0) == 4);
        Instrumentation.testInvocationCount(invocationCount + 6);
        Instrumentation.testFailureCount(2);
        Instrumentation.testRetryCount(4);
        test(retry1.end_opIdempotent(retry1.begin_opIdempotent(4)) == 8);
        Instrumentation.testInvocationCount(invocationCount + 7);
        Instrumentation.testFailureCount(2);
        Instrumentation.testRetryCount(8);
        out.println("ok");

        out.print("testing non-idempotent operation... ");
        try
        {
            retry1.opNotIdempotent(8);
            test(false);
        }
        catch(Ice.LocalException ex)
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
        catch(Ice.LocalException ex)
        {
        }
        Instrumentation.testInvocationCount(invocationCount + 9);
        Instrumentation.testFailureCount(4);
        Instrumentation.testRetryCount(8);
        out.println("ok");

        if(retry1.ice_getConnection() == null)
        {
            invocationCount = invocationCount + 10;
            out.print("testing system exception... ");
            try
            {
                retry1.opSystemException();
                test(false);
            }
            catch(SystemFailure ex)
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
            catch(SystemFailure ex)
            {
            }
            Instrumentation.testInvocationCount(invocationCount + 2);
            Instrumentation.testFailureCount(6);
            Instrumentation.testRetryCount(8);
            out.println("ok");
        }

        return retry1;
    }
}
