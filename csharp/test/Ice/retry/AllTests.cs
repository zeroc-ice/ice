// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.IO;
using System.Text;
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

        public static IRetryPrx Run(TestHelper helper, Communicator communicator, bool colocated)
        {
           bool ice1 = helper.Protocol == Protocol.Ice1;

            TextWriter output = helper.Output;

            var retry1 = IRetryPrx.Parse(helper.GetTestProxy("retry"), communicator);

            output.Write("calling regular operation with first proxy... ");
            output.Flush();
            Instrumentation.TestInvocationReset();
            retry1.Op(false);
            Instrumentation.TestInvocationCount(1);
            Instrumentation.TestFailureCount(0);
            Instrumentation.TestRetryCount(0);
            output.WriteLine("ok");

            output.Write("calling operation to kill connection with second proxy... ");
            output.Flush();
            Instrumentation.TestInvocationReset();
            try
            {
                retry1.Op(true);
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
            // Instrumentation.TestFailureCount(1);
            Instrumentation.TestRetryCount(0);
            output.WriteLine("ok");

            output.Write("calling regular operation with first proxy again... ");
            output.Flush();
            Instrumentation.TestInvocationReset();
            retry1.Op(false);
            Instrumentation.TestInvocationCount(1);
            Instrumentation.TestFailureCount(0);
            Instrumentation.TestRetryCount(0);
            output.WriteLine("ok");

            output.Write("calling regular AMI operation with first proxy... ");
            Instrumentation.TestInvocationReset();
            retry1.OpAsync(false).Wait();
            Instrumentation.TestInvocationCount(1);
            Instrumentation.TestFailureCount(0);
            Instrumentation.TestRetryCount(0);
            output.WriteLine("ok");

            output.Write("calling AMI operation to kill connection with second proxy... ");
            Instrumentation.TestInvocationReset();
            try
            {
                retry1.OpAsync(true).Wait();
                TestHelper.Assert(false);
            }
            catch (AggregateException ex)
            {
                TestHelper.Assert(ex.InnerException is ConnectionLostException ||
                                  ex.InnerException is UnhandledException);
            }

            Instrumentation.TestInvocationCount(1);
            // Instrumentation.TestFailureCount(1);
            Instrumentation.TestRetryCount(0);
            output.WriteLine("ok");

            output.Write("calling regular AMI operation with first proxy again... ");
            Instrumentation.TestInvocationReset();
            retry1.OpAsync(false).Wait();
            Instrumentation.TestInvocationCount(1);
            Instrumentation.TestFailureCount(0);
            Instrumentation.TestRetryCount(0);
            output.WriteLine("ok");

            output.Write("testing non-idempotent operation... ");
            Instrumentation.TestInvocationReset();
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
            Instrumentation.TestInvocationReset();
            try
            {
                retry1.OpSystemException();
                TestHelper.Assert(false);
            }
            catch (SystemFailure)
            {
            }
            Instrumentation.TestInvocationCount(1);
            Instrumentation.TestFailureCount(0);
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
            Instrumentation.TestFailureCount(0);
            Instrumentation.TestRetryCount(0);
            output.WriteLine("ok");

            if (!ice1)
            {
                output.Write("testing cancellation and retries... ");
                output.Flush();
                Instrumentation.TestInvocationReset();
                try
                {
                    // No more than 2 retries before timeout kicks-in
                    using var cancel = new CancellationTokenSource(500);
                    retry1.OpIdempotent(4, cancel: cancel.Token);
                    TestHelper.Assert(false);
                }
                catch (OperationCanceledException)
                {
                    Instrumentation.TestRetryCount(2);
                    retry1.OpIdempotent(-1);
                }
                output.WriteLine("ok");

                output.Write("testing retry after delay... ");
                output.Flush();
                Instrumentation.TestInvocationReset();
                try
                {
                    // No retries before timeout kicks-in
                    using var cancel = new CancellationTokenSource(500);
                    retry1.OpAfterDelay(2, 600, cancel: cancel.Token);
                    TestHelper.Assert(false);
                }
                catch (OperationCanceledException)
                {
                    Instrumentation.TestRetryCount(0);
                    retry1.OpAfterDelay(-1, 0);
                }

                {
                    Instrumentation.TestInvocationReset();
                    // No retries before timeout kicks-in
                    using var cancel = new CancellationTokenSource(500);
                    int n = retry1.OpAfterDelay(4, 50, cancel: cancel.Token);
                    Instrumentation.TestRetryCount(4);
                    retry1.OpAfterDelay(-1, 0);
                    TestHelper.Assert(n == 4);
                }

                {
                    // No more than 5 invocation attempts with the default settings
                    Instrumentation.TestInvocationReset();
                    // No retries before timeout kicks-in
                    using var cancel = new CancellationTokenSource(500);
                    try
                    {
                        retry1.OpAfterDelay(5, 50, cancel: cancel.Token);
                        TestHelper.Assert(false);
                    }
                    catch (SystemFailure)
                    {
                        Instrumentation.TestRetryCount(4);
                        retry1.OpAfterDelay(-1, 0);
                    }
                }
                output.WriteLine("ok");

                if (!colocated)
                {
                    output.Write("testing retry other replica... ");
                    output.Flush();
                    // Build a multi-endpoint proxy by hand.
                    // TODO: should the TestHelper help with that?
                    var sb = new StringBuilder(helper.GetTestProxy("replicated"));
                    sb.Append("?alt-endpoint=");
                    sb.Append(helper.Host.Contains(":") ? $"[{helper.Host}]" : helper.Host);
                    sb.Append(':');
                    sb.Append(helper.BasePort + 1);

                    IReplicatedPrx? replicated = IReplicatedPrx.Parse(sb.ToString(), communicator).Clone(
                        endpointSelection: EndpointSelectionType.Ordered);

                    replicated.IcePing();
                    TestHelper.Assert(((IPConnection)replicated.GetCachedConnection()!).RemoteEndpoint!.Port ==
                                      helper.BasePort);

                    Instrumentation.TestInvocationReset();
                    replicated.OtherReplica();
                    Instrumentation.TestRetryCount(1);
                    TestHelper.Assert(((IPConnection)replicated.GetCachedConnection()!).RemoteEndpoint!.Port ==
                                      helper.BasePort + 1);

                    try
                    {
                        var nonreplicated = INonReplicatedPrx.Parse(helper.GetTestProxy("replicated"), communicator);

                        nonreplicated.IcePing();
                        TestHelper.Assert(((IPConnection)nonreplicated.GetCachedConnection()!).RemoteEndpoint!.Port ==
                                          helper.BasePort);
                        nonreplicated.OtherReplica();
                        TestHelper.Assert(false);
                    }
                    catch (SystemFailure)
                    {
                    }
                    output.WriteLine("ok");
                }
            }
            return retry1;
        }
    }
}
