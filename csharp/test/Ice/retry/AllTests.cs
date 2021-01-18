// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Retry
{
    public class Bidir : IBidir
    {
        private int _n;
        public void AfterDelay(int n, Current current, CancellationToken cancel)
        {
            if (++_n < n)
            {
                throw new ObjectNotExistException(RetryPolicy.AfterDelay(TimeSpan.FromMilliseconds(10)));
            }
            _n = 0;
        }

        public void OtherReplica(Current current, CancellationToken cancel) =>
            throw new ObjectNotExistException(RetryPolicy.OtherReplica);
    }
    public static class AllTests
    {
        public static async Task RunAsync(TestHelper helper, bool colocated)
        {
            Communicator communicator = helper.Communicator;
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
            Instrumentation.TestFailureCount(1);
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
            Instrumentation.TestFailureCount(1);
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
            Instrumentation.TestFailureCount(1);
            Instrumentation.TestRetryCount(0);
            output.WriteLine("ok");

            if (!colocated)
            {
                output.Write("testing retry with fixed reference... ");
                output.Flush();
                var adapter = communicator.CreateObjectAdapter(protocol: ice1 ? Protocol.Ice1 : Protocol.Ice2);
                var bidir = adapter.AddWithUUID(new Bidir(), IBidirPrx.Factory);
                (await retry1.GetConnectionAsync()).Adapter = adapter;
                retry1.OpBidirRetry(bidir);

                output.WriteLine("ok");
            }

            if (!ice1)
            {
                output.Write("testing cancellation and retries... ");
                output.Flush();
                Instrumentation.TestInvocationReset();
                try
                {
                    // No more than 2 retries before timeout kicks-in
                    retry1.Clone(invocationTimeout: TimeSpan.FromMilliseconds(500)).OpIdempotent(4);
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
                    retry1.Clone(invocationTimeout: TimeSpan.FromMilliseconds(400)).OpAfterDelay(2, 600);
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
                    int n = retry1.Clone(invocationTimeout: TimeSpan.FromMilliseconds(500)).OpAfterDelay(4, 50);
                    Instrumentation.TestRetryCount(4);
                    retry1.OpAfterDelay(-1, 0);
                    TestHelper.Assert(n == 4);
                }

                {
                    // No more than 5 invocation attempts with the default settings
                    Instrumentation.TestInvocationReset();
                    // No retries before timeout kicks-in
                    try
                    {
                        retry1.Clone(invocationTimeout: TimeSpan.FromMilliseconds(500)).OpAfterDelay(5, 50);
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

                    IReplicatedPrx? replicated = IReplicatedPrx.Parse(sb.ToString(), communicator);

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

                    output.Write("testing retry request size max... ");
                    output.Flush();
                    {
                        Dictionary<string, string> properties = communicator.GetProperties();
                        properties["Ice.RetryRequestMaxSize"] = "1024";
                        await using var communicator2 = new Communicator(properties);
                        var retry2 = IRetryPrx.Parse(helper.GetTestProxy("retry"), communicator2);

                        byte[] data = Enumerable.Range(0, 1024).Select(i => (byte)i).ToArray();

                        retry1.OpWithData(1, 0, data); // Succeed no retry request size limit

                        try
                        {
                            retry2.OpWithData(1, 0, data); // Fails because retry request size limit
                            TestHelper.Assert(false);
                        }
                        catch (SystemFailure)
                        {
                            // Expected
                            retry2.OpWithData(0, 0, Array.Empty<byte>()); // Reset the counter
                        }
                    }
                    output.WriteLine("ok");

                    output.Write("testing retry buffer size max... ");
                    output.Flush();
                    {
                        Dictionary<string, string> properties = communicator.GetProperties();
                        properties["Ice.RetryBufferMaxSize"] = "2048";
                        await using var communicator2 = new Communicator(properties);
                        var retry2 = IRetryPrx.Parse(helper.GetTestProxy("retry"), communicator2);

                        byte[] data = Enumerable.Range(0, 1024).Select(i => (byte)i).ToArray();

                        // Use two connections to simulate two concurrent retries, the first should succeed
                        // and the second should fail because the buffer size max.
                        Task t1 = retry2.Clone(label: "conn-1").OpWithDataAsync(2, 1000, data);
                        Thread.Sleep(100); // Ensure the first request it is send before the second request
                        Task t2 = retry2.Clone(label: "conn-2").OpWithDataAsync(2, 0, data);

                        // T1 succeed, T2 Fail because buffer size max
                        t1.Wait();
                        try
                        {
                            t2.Wait();
                            TestHelper.Assert(false);
                        }
                        catch (AggregateException ex)
                        {
                            // expected
                            TestHelper.Assert(ex.InnerException is SystemFailure);
                            retry2.OpWithData(0, 0, Array.Empty<byte>()); // Reset the counter
                        }
                        retry2.Clone(label: "conn-1").OpWithData(2, 100, data);
                    }
                    output.WriteLine("ok");
                }
            }
            await retry1.ShutdownAsync();
        }
    }
}
