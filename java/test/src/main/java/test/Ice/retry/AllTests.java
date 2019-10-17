//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.retry;

import java.io.PrintWriter;

import test.Ice.retry.Test.RetryPrx;

public class AllTests
{
    private static void test(boolean b)
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

        public synchronized void check()
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

        public synchronized void called()
        {
            assert(!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    public static RetryPrx allTests(test.TestHelper helper,
                                    com.zeroc.Ice.Communicator communicator,
                                    com.zeroc.Ice.Communicator communicator2,
                                    Instrumentation instrumentation,
                                    String ref)
    {
        PrintWriter out = helper.getWriter();
        out.print("testing stringToProxy... ");
        out.flush();
        com.zeroc.Ice.ObjectPrx base1 = communicator.stringToProxy(ref);
        test(base1 != null);
        com.zeroc.Ice.ObjectPrx base2 = communicator.stringToProxy(ref);
        test(base2 != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        RetryPrx retry1 = RetryPrx.checkedCast(base1);
        test(retry1 != null);
        test(retry1.equals(base1));
        RetryPrx retry2 = RetryPrx.checkedCast(base2);
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
        catch(com.zeroc.Ice.UnknownLocalException ex)
        {
            // Expected with collocation
        }
        catch(com.zeroc.Ice.ConnectionLostException ex)
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

        Callback cb1 = new Callback();
        Callback cb2 = new Callback();

        out.print("calling regular AMI operation with first proxy... ");
        retry1.opAsync(false).whenComplete((result, ex) ->
            {
                test(ex == null);
                cb1.called();
            });
        cb1.check();
        instrumentation.testInvocationCount(1);
        instrumentation.testFailureCount(0);
        instrumentation.testRetryCount(0);
        out.println("ok");

        out.print("calling AMI operation to kill connection with second proxy... ");
        retry2.opAsync(true).whenComplete((result, ex) ->
            {
                test(ex != null && (ex instanceof com.zeroc.Ice.ConnectionLostException ||
                                    ex instanceof com.zeroc.Ice.UnknownLocalException));
                cb2.called();
            });
        cb2.check();
        instrumentation.testInvocationCount(1);
        instrumentation.testFailureCount(1);
        instrumentation.testRetryCount(0);
        out.println("ok");

        out.print("calling regular AMI operation with first proxy again... ");
        retry1.opAsync(false).whenComplete((result, ex) ->
            {
                test(ex == null);
                cb1.called();
            });
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
        test(retry1.opIdempotentAsync(4).join() == 4);
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
        catch(com.zeroc.Ice.LocalException ex)
        {
        }
        instrumentation.testInvocationCount(1);
        instrumentation.testFailureCount(1);
        instrumentation.testRetryCount(0);
        try
        {
            retry1.opNotIdempotentAsync().join();
            test(false);
        }
        catch(java.util.concurrent.CompletionException ex)
        {
            test(ex.getCause() instanceof com.zeroc.Ice.LocalException);
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
                retry1.opSystemExceptionAsync().join();
                test(false);
            }
            catch(java.util.concurrent.CompletionException ex)
            {
                test(ex.getCause() instanceof SystemFailure);
            }
            instrumentation.testInvocationCount(1);
            instrumentation.testFailureCount(1);
            instrumentation.testRetryCount(0);
            out.println("ok");
        }

        {
            out.print("testing invocation timeout and retries... ");
            out.flush();

            retry2 = RetryPrx.checkedCast(communicator2.stringToProxy(retry1.toString()));
            try
            {
                // No more than 2 retries before timeout kicks-in
                retry2.ice_invocationTimeout(500).opIdempotent(4);
                test(false);
            }
            catch(com.zeroc.Ice.InvocationTimeoutException ex)
            {
                instrumentation.testRetryCount(2);
                retry2.opIdempotent(-1); // Reset the counter
                instrumentation.testRetryCount(-1);
            }
            try
            {
                // No more than 2 retries before timeout kicks-in
                RetryPrx prx = retry2.ice_invocationTimeout(500);
                prx.opIdempotentAsync(4).join();
                test(false);
            }
            catch(java.util.concurrent.CompletionException ex)
            {
                test(ex.getCause() instanceof com.zeroc.Ice.InvocationTimeoutException);
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
                catch(com.zeroc.Ice.TimeoutException ex)
                {
                }
                instrumentation.testRetryCount(4);
            }
            out.println("ok");
        }

        return retry1;
    }
}
