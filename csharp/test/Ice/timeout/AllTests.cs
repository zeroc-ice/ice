// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Timeout
{
    public static class AllTests
    {
        public static async Task RunAsync(TestHelper helper)
        {
            Communicator communicator = helper.Communicator;

            var controller = IControllerPrx.Parse(helper.GetTestProxy("controller", 1), communicator);
            try
            {
                await RunWithControllerAsync(helper, controller);
            }
            catch
            {
                // Ensure the adapter is not in the holding state when an unexpected exception occurs to prevent
                // the test from hanging on exit in case a connection which disables timeouts is still opened.
                controller.ResumeAdapter();
                throw;
            }
        }

        public static async Task RunWithControllerAsync(TestHelper helper, IControllerPrx controller)
        {
            Communicator communicator = helper.Communicator;

            bool ice1 = TestHelper.GetTestProtocol(communicator.GetProperties()) == Protocol.Ice1;

            var timeout = ITimeoutPrx.Parse(helper.GetTestProxy("timeout", 0), communicator);

            System.IO.TextWriter output = helper.Output;
            output.Write("testing connect timeout... ");
            output.Flush();
            {
                Dictionary<string, string> properties = communicator.GetProperties();
                properties["Ice.ConnectTimeout"] = "100ms";
                await using var comm = new Communicator(properties);

                var to = ITimeoutPrx.Parse(helper.GetTestProxy("timeout", 0), comm);

                // Expect ConnectTimeoutException.
                controller.HoldAdapter(-1);
                try
                {
                    to.Op();
                    TestHelper.Assert(false);
                }
                catch (ConnectTimeoutException)
                {
                    // Expected.
                }
                controller.ResumeAdapter();
                timeout.Op(); // Ensure adapter is active.
            }
            {
                // Expect success.
                controller.HoldAdapter(100);
                timeout.Op();
            }
            output.WriteLine("ok");

            output.Write("testing invocation timeout... ");
            output.Flush();
            {
                timeout.IcePing(); // Makes sure a working connection is associated with the proxy
                Connection connection = await timeout.GetConnectionAsync();
                try
                {
                    timeout.Clone(invocationTimeout: TimeSpan.FromMilliseconds(100)).SleepAsync(1000).Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ex) when (ex.InnerException is OperationCanceledException)
                {
                }
                timeout.IcePing();

                TestHelper.Assert(connection == await timeout.GetConnectionAsync());
                try
                {
                    timeout.Clone(invocationTimeout: TimeSpan.FromMilliseconds(1000)).SleepAsync(100).Wait();
                }
                catch (AggregateException ex) when (ex.InnerException is OperationCanceledException)
                {
                    TestHelper.Assert(false);
                }
                TestHelper.Assert(connection == await timeout.GetConnectionAsync());

                try
                {
                    timeout.Clone(invocationTimeout: TimeSpan.FromMilliseconds(100)).Sleep(1000);
                    TestHelper.Assert(false);
                }
                catch (OperationCanceledException)
                {
                }
            }
            output.WriteLine("ok");

            output.Write("testing close timeout... ");
            output.Flush();
            {
                Dictionary<string, string> properties = communicator.GetProperties();
                properties["Ice.CloseTimeout"] = "100ms";
                await using var comm = new Communicator(properties);

                var to = ITimeoutPrx.Parse(helper.GetTestProxy("timeout", 0), comm);

                Connection connection = await to.GetConnectionAsync();
                Connection connection2 = await timeout.GetConnectionAsync(); // No close timeout

                TestHelper.Assert(connection != null && connection2 != null);

                controller.HoldAdapter(-1);

                // Make sure there's no ReadAsync pending
                _ = to.IcePingAsync();
                _ = timeout.IcePingAsync();

                var semaphore = new System.Threading.SemaphoreSlim(0);
                connection.Closed += (sender, args) => semaphore.Release();
                _ = connection.GoAwayAsync();
                TestHelper.Assert(semaphore.Wait(500));

                connection2.Closed += (sender, args) => semaphore.Release();
                _ = connection2.GoAwayAsync();
                TestHelper.Assert(!semaphore.Wait(500));

                controller.ResumeAdapter();
                timeout.Op(); // Ensure adapter is active.
            }
            output.WriteLine("ok");

            if (!ice1)
            {
                output.Write("testing deadlines... ");
                output.Flush();
                {
                    var comm1 = new Communicator(communicator.GetProperties());

                    comm1.DefaultInvocationInterceptors = ImmutableList.Create<InvocationInterceptor>(
                            (target, request, next, cancel) =>
                            {
                                request.BinaryContextOverride.Add(10, ostr =>
                                {
                                    var deadline = (request.Deadline - DateTime.UnixEpoch).TotalMilliseconds;
                                    ostr.WriteVarLong((long)deadline);
                                });
                                return next(target, request, cancel);
                            });

                    for (int i = 1000; i < 5000;)
                    {
                        i += 33;
                        ITimeoutPrx to = ITimeoutPrx.Parse(helper.GetTestProxy("timeout", 0), comm1).Clone(
                            invocationTimeout: TimeSpan.FromMilliseconds(i));
                        TestHelper.Assert(to.CheckDeadline());
                    }
                }
                output.WriteLine("ok");
            }
            controller.Shutdown();
        }
    }
}
