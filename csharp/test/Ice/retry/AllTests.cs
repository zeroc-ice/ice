// Copyright (c) ZeroC, Inc.

namespace Ice.retry
{
        public class AllTests : global::Test.AllTests
        {
            public static async Task<Test.RetryPrx> allTests(global::Test.TestHelper helper,
                Communicator communicator,
                Communicator communicator2,
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

                output.Write("calling regular AMI operation with first proxy... ");
                await retry1.opAsync(false);
                Instrumentation.testInvocationCount(1);
                Instrumentation.testFailureCount(0);
                Instrumentation.testRetryCount(0);
                output.WriteLine("ok");

                output.Write("calling AMI operation to kill connection with second proxy... ");
                try
                {
                    await retry2.opAsync(true);
                    test(false);
                }
                catch (ConnectionLostException)
                {
                }
                catch (UnknownLocalException)
                {
                }

                Instrumentation.testInvocationCount(1);
                Instrumentation.testFailureCount(1);
                Instrumentation.testRetryCount(0);
                output.WriteLine("ok");

                output.Write("calling regular AMI operation with first proxy again... ");
                await retry1.opAsync(false);
                Instrumentation.testInvocationCount(1);
                Instrumentation.testFailureCount(0);
                Instrumentation.testRetryCount(0);
                output.WriteLine("ok");

                output.Write("testing idempotent operation... ");
                test(retry1.opIdempotent(4) == 4);
                Instrumentation.testInvocationCount(1);
                Instrumentation.testFailureCount(0);
                Instrumentation.testRetryCount(4);
                test(await retry1.opIdempotentAsync(4) == 4);
                Instrumentation.testInvocationCount(1);
                Instrumentation.testFailureCount(0);
                Instrumentation.testRetryCount(4);
                output.WriteLine("ok");

                if (retry1.ice_getCachedConnection() != null)
                {
                    output.Write("testing non-idempotent operation with bi-dir proxy... ");
                    try
                    {
                        ((Test.RetryPrx)retry1.ice_fixed(retry1.ice_getCachedConnection())).opIdempotent(4);
                    }
                    catch (Ice.Exception)
                    {
                    }
                    Instrumentation.testInvocationCount(1);
                    Instrumentation.testFailureCount(1);
                    Instrumentation.testRetryCount(0);
                    test(retry1.opIdempotent(4) == 4);
                    Instrumentation.testInvocationCount(1);
                    Instrumentation.testFailureCount(0);
                    // It succeeded after 3 retry because of the failed opIdempotent on the fixed proxy above
                    Instrumentation.testRetryCount(3);
                    output.WriteLine("ok");
                }

                output.Write("testing non-idempotent operation... ");
                try
                {
                    retry1.opNotIdempotent();
                    test(false);
                }
                catch (LocalException)
                {
                }
                Instrumentation.testInvocationCount(1);
                Instrumentation.testFailureCount(1);
                Instrumentation.testRetryCount(0);
                try
                {
                    await retry1.opNotIdempotentAsync();
                    test(false);
                }
                catch (LocalException)
                {
                }
                Instrumentation.testInvocationCount(1);
                Instrumentation.testFailureCount(1);
                Instrumentation.testRetryCount(0);
                output.WriteLine("ok");

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
                    catch (InvocationTimeoutException)
                    {
                        Instrumentation.testRetryCount(2);
                        retry2.opIdempotent(-1); // Reset the counter
                        Instrumentation.testRetryCount(-1);
                    }
                    try
                    {
                        // No more than 2 retries before timeout kicks-in
                        Test.RetryPrx prx = (Test.RetryPrx)retry2.ice_invocationTimeout(500);
                        await prx.opIdempotentAsync(4);
                        test(false);
                    }
                    catch (InvocationTimeoutException)
                    {
                        Instrumentation.testRetryCount(2);
                        retry2.opIdempotent(-1); // Reset the counter
                        Instrumentation.testRetryCount(-1);
                    }
                    output.WriteLine("ok");
                }
                return retry1;
            }
        }
    }

