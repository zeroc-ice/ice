//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.IO;
using System.Threading;
using Test;

namespace ZeroC.Ice.Test.Retry
{
    public class AllTests
    {
        private class Callback
        {
            private bool _called;
            private readonly object _mutex = new object();

            internal Callback() => _called = false;

            public void Check()
            {
                lock (_mutex)
                {
                    while (!_called)
                    {
                        Monitor.Wait(this);
                    }

                    _called = false;
                }
            }

            public void Called()
            {
                lock (_mutex)
                {
                    TestHelper.Assert(!_called);
                    _called = true;
                    Monitor.Pulse(this);
                }
            }
        }

        public static IRetryPrx Run(
            TestHelper helper,
            Communicator communicator,
            Communicator communicator2,
            string rf)
        {
            bool ice1 = communicator.DefaultProtocol == Protocol.Ice1; // TODO should come from helper

            Instrumentation.TestInvocationReset();

            TextWriter output = helper.GetWriter();
            output.Write("testing stringToProxy... ");
            output.Flush();
            var base1 = IObjectPrx.Parse(rf, communicator);
            var base2 = IObjectPrx.Parse(rf, communicator);
            output.WriteLine("ok");

            output.Write("testing checked cast... ");
            output.Flush();
            var retry1 = IRetryPrx.CheckedCast(base1);
            TestHelper.Assert(retry1 != null && retry1.Equals(base1));
            var retry2 = IRetryPrx.CheckedCast(base2);
            TestHelper.Assert(retry2 != null && retry2.Equals(base2));
            output.WriteLine("ok");

            output.Write("calling regular operation with first proxy... ");
            output.Flush();
            retry1.Op(false);
            output.WriteLine("ok");

            Instrumentation.TestInvocationCount(3);

            output.Write("calling operation to kill connection with second proxy... ");
            output.Flush();
            try
            {
                retry2.Op(true);
                TestHelper.Assert(false);
            }
            catch (UnhandledException)
            {
                // Expected with collocation
            }
            catch (ConnectionLostException)
            {
            }
            Instrumentation.TestInvocationCount(1);
            Instrumentation.TestFailureCount(1);
            Instrumentation.TestRetryCount(0);
            output.WriteLine("ok");

            output.Write("calling regular operation with first proxy again... ");
            output.Flush();
            retry1.Op(false);
            Instrumentation.TestInvocationCount(1);
            Instrumentation.TestFailureCount(0);
            Instrumentation.TestRetryCount(0);
            output.WriteLine("ok");

            output.Write("calling regular AMI operation with first proxy... ");
            retry1.OpAsync(false).Wait();
            Instrumentation.TestInvocationCount(1);
            Instrumentation.TestFailureCount(0);
            Instrumentation.TestRetryCount(0);
            output.WriteLine("ok");

            output.Write("calling AMI operation to kill connection with second proxy... ");
            try
            {
                retry2.OpAsync(true).Wait();
                TestHelper.Assert(false);
            }
            catch (AggregateException ex)
            {
                TestHelper.Assert(ex.InnerException is ConnectionLostException ||
                                  ex.InnerException is UnhandledException);
            }

            Instrumentation.TestInvocationCount(1);
            Instrumentation.TestFailureCount(1);
            Instrumentation.TestRetryCount(0);
            output.WriteLine("ok");

            output.Write("calling regular AMI operation with first proxy again... ");
            retry1.OpAsync(false).Wait();
            Instrumentation.TestInvocationCount(1);
            Instrumentation.TestFailureCount(0);
            Instrumentation.TestRetryCount(0);
            output.WriteLine("ok");

            // TODO: with ice1, we retry on UnhandledExceptions (!) thrown by the server when the operation is
            // idempotent. With ice2, no remote exception currently triggers any retry.
            if (ice1)
            {
                output.Write("testing idempotent operation... ");
                TestHelper.Assert(retry1.OpIdempotent(4) == 4);
                Instrumentation.TestInvocationCount(1);
                Instrumentation.TestFailureCount(0);
                Instrumentation.TestRetryCount(4);
                TestHelper.Assert(retry1.OpIdempotentAsync(4).Result == 4);
                Instrumentation.TestInvocationCount(1);
                Instrumentation.TestFailureCount(0);
                Instrumentation.TestRetryCount(4);
                output.WriteLine("ok");

                if (retry1.GetCachedConnection() != null)
                {
                    output.Write("testing idempotent operation with bi-dir proxy... ");
                    try
                    {
                        retry1.Clone(fixedConnection: retry1.GetCachedConnection()).OpIdempotent(4);
                    }
                    catch (UnhandledException)
                    {
                    }
                    Instrumentation.TestInvocationCount(1);
                    Instrumentation.TestFailureCount(1);

                    Instrumentation.TestRetryCount(0);
                    TestHelper.Assert(retry1.OpIdempotent(4) == 4);
                    Instrumentation.TestInvocationCount(1);
                    Instrumentation.TestFailureCount(0);
                    // It succeeded after 3 retry because of the failed opIdempotent on the fixed proxy above
                    Instrumentation.TestRetryCount(3);
                    output.WriteLine("ok");
                }
            }

            output.Write("testing non-idempotent operation... ");
            try
            {
                retry1.OpNotIdempotent();
                TestHelper.Assert(false);
            }
            catch (UnhandledException)
            {
            }
            Instrumentation.TestInvocationCount(1);
            if (ice1)
            {
                Instrumentation.TestFailureCount(1); // TODO: ice2
            }
            Instrumentation.TestRetryCount(0);
            try
            {
                retry1.OpNotIdempotentAsync().Wait();
                TestHelper.Assert(false);
            }
            catch (AggregateException ex)
            {
                TestHelper.Assert(ex.InnerException is UnhandledException);
            }
            Instrumentation.TestInvocationCount(1);
            if (ice1)
            {
                Instrumentation.TestFailureCount(1); // TODO: ice2
            }
            Instrumentation.TestRetryCount(0);
            output.WriteLine("ok");

            output.Write("testing system exception... "); // it's just a regular remote exception
            try
            {
                retry1.OpSystemException();
                TestHelper.Assert(false);
            }
            catch (SystemFailure)
            {
            }
            Instrumentation.TestInvocationCount(1);
            if (ice1)
            {
                Instrumentation.TestFailureCount(0); // TODO: ice2
            }
            Instrumentation.TestRetryCount(0);
            try
            {
                retry1.OpSystemExceptionAsync().Wait();
                TestHelper.Assert(false);
            }
            catch (AggregateException ex)
            {
                TestHelper.Assert(ex.InnerException is SystemFailure);
            }
            Instrumentation.TestInvocationCount(1);
            if (ice1)
            {
                Instrumentation.TestFailureCount(0); // TODO: ice2
            }
            Instrumentation.TestRetryCount(0);
            output.WriteLine("ok");

            if (ice1)
            {
                output.Write("testing invocation timeout and retries... ");
                output.Flush();

                retry2 = IRetryPrx.Parse(retry1.ToString()!, communicator2);
                try
                {
                    // No more than 2 retries before timeout kicks-in
                    retry2.Clone(invocationTimeout: 500).OpIdempotent(4);
                    TestHelper.Assert(false);
                }
                catch (TimeoutException)
                {
                    Instrumentation.TestRetryCount(2);
                    retry2.OpIdempotent(-1); // Reset the counter
                    Instrumentation.TestRetryCount(-1);
                }
                try
                {
                    // No more than 2 retries before timeout kicks-in
                    IRetryPrx prx = retry2.Clone(invocationTimeout: 500);
                    prx.OpIdempotentAsync(4).Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ex)
                {
                    TestHelper.Assert(ex.InnerException is TimeoutException);
                    Instrumentation.TestRetryCount(2);
                    retry2.OpIdempotent(-1); // Reset the counter
                    Instrumentation.TestRetryCount(-1);
                }
                output.WriteLine("ok");
            }
            return retry1;
        }
    }
}
