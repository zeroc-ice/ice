//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;
using System.Threading;

namespace Ice
{
    namespace retry
    {
        public class AllTests : global::Test.AllTests
        {
            private class Callback
            {
                internal Callback()
                {
                    _called = false;
                }

                public void check()
                {
                    lock (this)
                    {
                        while (!_called)
                        {
                            Monitor.Wait(this);
                        }

                        _called = false;
                    }
                }

                public void called()
                {
                    lock (this)
                    {
                        Debug.Assert(!_called);
                        _called = true;
                        Monitor.Pulse(this);
                    }
                }

                private bool _called;
            }

            static public Test.RetryPrx
            allTests(global::Test.TestHelper helper,
                     Ice.Communicator communicator,
                     Ice.Communicator communicator2,
                     string rf)
            {
                Instrumentation.testInvocationReset();

                var output = helper.getWriter();
                output.Write("testing stringToProxy... ");
                output.Flush();
                var base1 = communicator.stringToProxy(rf);
                test(base1 != null);
                var base2 = communicator.stringToProxy(rf);
                test(base2 != null);
                output.WriteLine("ok");

                output.Write("testing checked cast... ");
                output.Flush();
                Test.RetryPrx retry1 = Test.RetryPrxHelper.checkedCast(base1);
                test(retry1 != null);
                test(retry1.Equals(base1));
                Test.RetryPrx retry2 = Test.RetryPrxHelper.checkedCast(base2);
                test(retry2 != null);
                test(retry2.Equals(base2));
                output.WriteLine("ok");

                output.Write("calling regular operation with first proxy... ");
                output.Flush();
                retry1.op(false);
                output.WriteLine("ok");

                Instrumentation.testInvocationCount(3);

                output.Write("calling operation to kill connection with second proxy... ");
                output.Flush();
                try
                {
                    retry2.op(true);
                    test(false);
                }
                catch (Ice.UnknownLocalException)
                {
                    // Expected with collocation
                }
                catch (Ice.ConnectionLostException)
                {
                }
                Instrumentation.testInvocationCount(1);
                Instrumentation.testFailureCount(1);
                Instrumentation.testRetryCount(0);
                output.WriteLine("ok");

                output.Write("calling regular operation with first proxy again... ");
                output.Flush();
                retry1.op(false);
                Instrumentation.testInvocationCount(1);
                Instrumentation.testFailureCount(0);
                Instrumentation.testRetryCount(0);
                output.WriteLine("ok");

                Callback cb = new Callback();

                output.Write("calling regular AMI operation with first proxy... ");
                retry1.begin_op(false).whenCompleted(
                   () =>
                    {
                        cb.called();
                    },
                   (Ice.Exception ex) =>
                    {
                        test(false);
                    });
                cb.check();
                Instrumentation.testInvocationCount(1);
                Instrumentation.testFailureCount(0);
                Instrumentation.testRetryCount(0);
                output.WriteLine("ok");

                output.Write("calling AMI operation to kill connection with second proxy... ");
                retry2.begin_op(true).whenCompleted(
                   () =>
                    {
                        test(false);
                    },
                   (Ice.Exception ex) =>
                    {
                        test(ex is Ice.ConnectionLostException || ex is Ice.UnknownLocalException);
                        cb.called();
                    });
                cb.check();
                Instrumentation.testInvocationCount(1);
                Instrumentation.testFailureCount(1);
                Instrumentation.testRetryCount(0);
                output.WriteLine("ok");

                output.Write("calling regular AMI operation with first proxy again... ");
                retry1.begin_op(false).whenCompleted(
                   () =>
                    {
                        cb.called();
                    },
                   (Ice.Exception ex) =>
                    {
                        test(false);
                    });
                cb.check();
                Instrumentation.testInvocationCount(1);
                Instrumentation.testFailureCount(0);
                Instrumentation.testRetryCount(0);
                output.WriteLine("ok");

                output.Write("testing idempotent operation... ");
                test(retry1.opIdempotent(4) == 4);
                Instrumentation.testInvocationCount(1);
                Instrumentation.testFailureCount(0);
                Instrumentation.testRetryCount(4);
                test(retry1.end_opIdempotent(retry1.begin_opIdempotent(4)) == 4);
                Instrumentation.testInvocationCount(1);
                Instrumentation.testFailureCount(0);
                Instrumentation.testRetryCount(4);
                output.WriteLine("ok");

                output.Write("testing non-idempotent operation... ");
                try
                {
                    retry1.opNotIdempotent();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                Instrumentation.testInvocationCount(1);
                Instrumentation.testFailureCount(1);
                Instrumentation.testRetryCount(0);
                try
                {
                    retry1.end_opNotIdempotent(retry1.begin_opNotIdempotent());
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                Instrumentation.testInvocationCount(1);
                Instrumentation.testFailureCount(1);
                Instrumentation.testRetryCount(0);
                output.WriteLine("ok");

                if (retry1.ice_getConnection() == null)
                {
                    Instrumentation.testInvocationCount(1);

                    output.Write("testing system exception... ");
                    try
                    {
                        retry1.opSystemException();
                        test(false);
                    }
                    catch (SystemFailure)
                    {
                    }
                    Instrumentation.testInvocationCount(1);
                    Instrumentation.testFailureCount(1);
                    Instrumentation.testRetryCount(0);
                    try
                    {
                        retry1.end_opSystemException(retry1.begin_opSystemException());
                        test(false);
                    }
                    catch (SystemFailure)
                    {
                    }
                    Instrumentation.testInvocationCount(1);
                    Instrumentation.testFailureCount(1);
                    Instrumentation.testRetryCount(0);
                    output.WriteLine("ok");
                }

                {
                    output.Write("testing invocation timeout and retries... ");
                    output.Flush();

                    retry2 = Test.RetryPrxHelper.checkedCast(communicator2.stringToProxy(retry1.ToString()));
                    try
                    {
                        // No more than 2 retries before timeout kicks-in
                        ((Test.RetryPrx)retry2.ice_invocationTimeout(500)).opIdempotent(4);
                        test(false);
                    }
                    catch (Ice.InvocationTimeoutException)
                    {
                        Instrumentation.testRetryCount(2);
                        retry2.opIdempotent(-1); // Reset the counter
                        Instrumentation.testRetryCount(-1);
                    }
                    try
                    {
                        // No more than 2 retries before timeout kicks-in
                        Test.RetryPrx prx = (Test.RetryPrx)retry2.ice_invocationTimeout(500);
                        prx.end_opIdempotent(prx.begin_opIdempotent(4));
                        test(false);
                    }
                    catch (Ice.InvocationTimeoutException)
                    {
                        Instrumentation.testRetryCount(2);
                        retry2.opIdempotent(-1); // Reset the counter
                        Instrumentation.testRetryCount(-1);
                    }
                    if (retry1.ice_getConnection() != null)
                    {
                        // The timeout might occur on connection establishment or because of the sleep. What's
                        // important here is to make sure there are 4 retries and that no calls succeed to
                        // ensure retries with the old connection timeout semantics work.
                        Test.RetryPrx retryWithTimeout =
                            (Test.RetryPrx)retry1.ice_invocationTimeout(-2).ice_timeout(200);
                        try
                        {
                            retryWithTimeout.sleep(500);
                            test(false);
                        }
                        catch (Ice.TimeoutException)
                        {
                        }
                        Instrumentation.testRetryCount(4);
                    }
                    output.WriteLine("ok");
                }
                return retry1;
            }
        }
    }
}
