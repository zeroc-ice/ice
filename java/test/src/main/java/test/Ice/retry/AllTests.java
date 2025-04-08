// Copyright (c) ZeroC, Inc.

package test.Ice.retry;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ConnectionLostException;
import com.zeroc.Ice.InvocationTimeoutException;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.UnknownLocalException;

import test.Ice.retry.Test.RetryPrx;
import test.TestHelper;

import java.io.PrintWriter;
import java.util.concurrent.CompletionException;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    private static class Callback {
        Callback() {
            _called = false;
        }

        public synchronized void check() {
            while (!_called) {
                try {
                    wait();
                } catch (InterruptedException ex) {
                }
            }

            _called = false;
        }

        public synchronized void called() {
            assert (!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    public static RetryPrx allTests(
            TestHelper helper,
            Communicator communicator,
            Communicator communicator2,
            Instrumentation instrumentation,
            String ref) {
        PrintWriter out = helper.getWriter();
        out.print("testing stringToProxy... ");
        out.flush();
        ObjectPrx base1 = communicator.stringToProxy(ref);
        test(base1 != null);
        ObjectPrx base2 = communicator.stringToProxy(ref);
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
        try {
            retry2.op(true);
            test(false);
        } catch (UnknownLocalException ex) {
            // Expected with collocation
        } catch (ConnectionLostException ex) {
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
        retry1.opAsync(false)
            .whenComplete(
                (result, ex) -> {
                    test(ex == null);
                    cb1.called();
                });
        cb1.check();
        instrumentation.testInvocationCount(1);
        instrumentation.testFailureCount(0);
        instrumentation.testRetryCount(0);
        out.println("ok");

        out.print("calling AMI operation to kill connection with second proxy... ");
        retry2.opAsync(true)
            .whenComplete(
                (result, ex) -> {
                    test(
                        ex != null
                            && (ex instanceof ConnectionLostException
                            || ex
                            instanceof UnknownLocalException));
                    cb2.called();
                });
        cb2.check();
        instrumentation.testInvocationCount(1);
        instrumentation.testFailureCount(1);
        instrumentation.testRetryCount(0);
        out.println("ok");

        out.print("calling regular AMI operation with first proxy again... ");
        retry1.opAsync(false)
            .whenComplete(
                (result, ex) -> {
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

        if (retry1.ice_getCachedConnection() != null) {
            out.print("testing non-idempotent operation with bi-dir proxy... ");
            try {
                ((RetryPrx) retry1.ice_fixed(retry1.ice_getCachedConnection())).opIdempotent(4);
            } catch (LocalException ex) {
            }
            instrumentation.testInvocationCount(1);
            instrumentation.testFailureCount(1);
            instrumentation.testRetryCount(0);
            test(retry1.opIdempotent(4) == 4);
            instrumentation.testInvocationCount(1);
            instrumentation.testFailureCount(0);
            // It succeeded after 3 retry because of the failed opIdempotent on the fixed proxy
            // above
            instrumentation.testRetryCount(3);
            out.println("ok");
        }

        out.print("testing non-idempotent operation... ");
        try {
            retry1.opNotIdempotent();
            test(false);
        } catch (LocalException ex) {
        }
        instrumentation.testInvocationCount(1);
        instrumentation.testFailureCount(1);
        instrumentation.testRetryCount(0);
        try {
            retry1.opNotIdempotentAsync().join();
            test(false);
        } catch (CompletionException ex) {
            test(ex.getCause() instanceof LocalException);
        }
        instrumentation.testInvocationCount(1);
        instrumentation.testFailureCount(1);
        instrumentation.testRetryCount(0);
        out.println("ok");

        {
            out.print("testing invocation timeout and retries... ");
            out.flush();

            retry2 = RetryPrx.checkedCast(communicator2.stringToProxy(retry1.toString()));
            try {
                // No more than 2 retries before timeout kicks-in
                retry2.ice_invocationTimeout(500).opIdempotent(4);
                test(false);
            } catch (InvocationTimeoutException ex) {
                instrumentation.testRetryCount(2);
                retry2.opIdempotent(-1); // Reset the counter
                instrumentation.testRetryCount(-1);
            }
            try {
                // No more than 2 retries before timeout kicks-in
                RetryPrx prx = retry2.ice_invocationTimeout(500);
                prx.opIdempotentAsync(4).join();
                test(false);
            } catch (CompletionException ex) {
                test(ex.getCause() instanceof InvocationTimeoutException);
                instrumentation.testRetryCount(2);
                retry2.opIdempotent(-1); // Reset the counter
                instrumentation.testRetryCount(-1);
            }
            out.println("ok");
        }

        return retry1;
    }

    private AllTests() {
    }
}
