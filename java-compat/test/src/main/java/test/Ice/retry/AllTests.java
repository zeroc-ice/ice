//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
    allTests(Ice.Communicator communicator, Ice.Communicator communicator2, PrintWriter out,
             Instrumentation instrumentation, String ref)
    {
        out.print("testing stringToProxy... ");
        out.flush();
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

        instrumentation.testInvocationCount(3);

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
        instrumentation.testInvocationCount(1);
        instrumentation.testFailureCount(1);
        instrumentation.testRetryCount(0);
        out.println("ok");

        out.print("calling regular operation with first proxy again... ");
        out.flush();
        retry1.op(false);
        instrumentation.testInvocationCount(1);
        instrumentation.testFailureCount(0);
        instrumentation.testRetryCount(0);
        out.println("ok");

        AMIRegular cb1 = new AMIRegular();
        AMIException cb2 = new AMIException();

        out.print("calling regular AMI operation with first proxy... ");
        retry1.begin_op(false, cb1);
        cb1.check();
        instrumentation.testInvocationCount(1);
        instrumentation.testFailureCount(0);
        instrumentation.testRetryCount(0);
        out.println("ok");

        out.print("calling AMI operation to kill connection with second proxy... ");
        retry2.begin_op(true, cb2);
        cb2.check();
        instrumentation.testInvocationCount(1);
        instrumentation.testFailureCount(1);
        instrumentation.testRetryCount(0);
        out.println("ok");

        out.print("calling regular AMI operation with first proxy again... ");
        retry1.begin_op(false, cb1);
        cb1.check();
        instrumentation.testInvocationCount(1);
        instrumentation.testFailureCount(0);
        instrumentation.testRetryCount(0);
        out.println("ok");

        out.print("testing idempotent operation... ");
        test(retry1.opIdempotent(4) == 4);
        instrumentation.testInvocationCount(1);
        instrumentation.testFailureCount(0);
        instrumentation.testRetryCount(4);
        test(retry1.end_opIdempotent(retry1.begin_opIdempotent(4)) == 4);
        instrumentation.testInvocationCount(1);
        instrumentation.testFailureCount(0);
        instrumentation.testRetryCount(4);
        out.println("ok");

        out.print("testing non-idempotent operation... ");
        try
        {
            retry1.opNotIdempotent();
            test(false);
        }
        catch(Ice.LocalException ex)
        {
        }
        instrumentation.testInvocationCount(1);
        instrumentation.testFailureCount(1);
        instrumentation.testRetryCount(0);
        try
        {
            retry1.end_opNotIdempotent(retry1.begin_opNotIdempotent());
            test(false);
        }
        catch(Ice.LocalException ex)
        {
        }
        instrumentation.testInvocationCount(1);
        instrumentation.testFailureCount(1);
        instrumentation.testRetryCount(0);
        out.println("ok");

        if(retry1.ice_getConnection() == null)
        {
            instrumentation.testInvocationCount(1);

            out.print("testing system exception... ");
            try
            {
                retry1.opSystemException();
                test(false);
            }
            catch(SystemFailure ex)
            {
            }
            instrumentation.testInvocationCount(1);
            instrumentation.testFailureCount(1);
            instrumentation.testRetryCount(0);
            try
            {
                retry1.end_opSystemException(retry1.begin_opSystemException());
                test(false);
            }
            catch(SystemFailure ex)
            {
            }
            instrumentation.testInvocationCount(1);
            instrumentation.testFailureCount(1);
            instrumentation.testRetryCount(0);
            out.println("ok");
        }

        {
            out.print("testing timeouts and retries... ");
            out.flush();
            retry2 = RetryPrxHelper.checkedCast(communicator2.stringToProxy(retry1.toString()));
            try
            {
                // No more than 2 retries before timeout kicks-in
                ((RetryPrx)retry2.ice_invocationTimeout(500)).opIdempotent(4);
                test(false);
            }
            catch(Ice.InvocationTimeoutException ex)
            {
                instrumentation.testRetryCount(2);
                retry2.opIdempotent(-1); // Reset the counter
                instrumentation.testRetryCount(-1);
            }
            try
            {
                // No more than 2 retries before timeout kicks-in
                RetryPrx prx = (RetryPrx)retry2.ice_invocationTimeout(500);
                prx.end_opIdempotent(prx.begin_opIdempotent(4));
                test(false);
            }
            catch(Ice.InvocationTimeoutException ex)
            {
                instrumentation.testRetryCount(2);
                retry2.opIdempotent(-1); // Reset the counter
                instrumentation.testRetryCount(-1);
            }
            if(retry1.ice_getConnection() != null)
            {
                // The timeout might occur on connection establishment or because of the sleep. What's
                // important here is to make sure there are 4 retries and that no calls succeed to
                // ensure retries with the old connection timeout semantics work.
                RetryPrx retryWithTimeout = (RetryPrx)retry1.ice_invocationTimeout(-2).ice_timeout(200);
                try
                {
                    retryWithTimeout.sleep(500);
                    test(false);
                }
                catch(Ice.TimeoutException ex)
                {
                }
                instrumentation.testRetryCount(4);
            }
            out.println("ok");
        }

        return retry1;
    }
}
