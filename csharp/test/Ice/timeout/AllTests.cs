// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Threading;
using System.Collections.Generic;
using Test;

namespace ZeroC.Ice.Test.Timeout
{
    public class AllTests
    {
        public static void Run(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator;
            TestHelper.Assert(communicator != null);
            var controller = IControllerPrx.Parse(helper.GetTestProxy("controller", 1), communicator);
            try
            {
                RunWithController(helper, controller);
            }
            catch
            {
                // Ensure the adapter is not in the holding state when an unexpected exception occurs to prevent
                // the test from hanging on exit in case a connection which disables timeouts is still opened.
                controller.ResumeAdapter();
                throw;
            }
        }

        public static void RunWithController(TestHelper helper, IControllerPrx controller)
        {
            Communicator? communicator = helper.Communicator;
            TestHelper.Assert(communicator != null);

            var timeout = ITimeoutPrx.Parse(helper.GetTestProxy("timeout", 0), communicator);

            System.IO.TextWriter output = helper.Output;
            output.Write("testing connect timeout... ");
            output.Flush();
            {
                Dictionary<string, string>? properties = communicator.GetProperties();
                properties["Ice.ConnectTimeout"] = "100ms";
                using var comm = new Communicator(properties);

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

            // The sequence needs to be large enough to fill the write/recv buffers
            byte[] seq = new byte[2000000];

            output.Write("testing connection timeout... ");
            output.Flush();
            {
                // Expect TimeoutException.
                controller.HoldAdapter(-1);
                timeout.GetConnection()!.Acm = new Acm(TimeSpan.FromMilliseconds(50),
                                                       AcmClose.OnInvocationAndIdle,
                                                       AcmHeartbeat.Off);
                try
                {
                    timeout.SendData(seq);
                    TestHelper.Assert(false);
                }
                catch (ConnectionTimeoutException)
                {
                    // Expected.
                }
                controller.ResumeAdapter();
                timeout.Op(); // Ensure adapter is active.
            }
            {
                // Expect success.
                controller.HoldAdapter(100);
                try
                {
                    timeout.SendData(new byte[1000000]);
                }
                catch (ConnectionTimeoutException)
                {
                    TestHelper.Assert(false);
                }
            }
            output.WriteLine("ok");

            output.Write("testing invocation timeout... ");
            output.Flush();
            {
                timeout.IcePing(); // Makes sure a working connection is associated with the proxy
                Connection? connection = timeout.GetConnection();
                try
                {
                    using var timeoutTokenSource = new CancellationTokenSource(TimeSpan.FromMilliseconds(100));
                    timeout.SleepAsync(1000, cancel: timeoutTokenSource.Token).Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ex) when (ex.InnerException is OperationCanceledException)
                {
                }
                timeout.IcePing();

                TestHelper.Assert(connection == timeout.GetConnection());
                try
                {
                    using var timeoutTokenSource = new CancellationTokenSource(TimeSpan.FromMilliseconds(1000));
                    timeout.SleepAsync(100, cancel: timeoutTokenSource.Token).Wait();
                }
                catch (AggregateException ex) when (ex.InnerException is OperationCanceledException)
                {
                    TestHelper.Assert(false);
                }
                TestHelper.Assert(connection == timeout.GetConnection());
            }
            output.WriteLine("ok");

            output.Write("testing close timeout... ");
            output.Flush();
            {
                Dictionary<string, string> properties = communicator.GetProperties();
                properties["Ice.CloseTimeout"] = "100ms";
                using var comm = new Communicator(properties);

                var to = ITimeoutPrx.Parse(helper.GetTestProxy("timeout", 0), comm);

                Connection? connection = to.GetConnection();
                Connection? connection2 = timeout.GetConnection(); // No close timeout

                TestHelper.Assert(connection != null && connection2 != null);

                controller.HoldAdapter(-1);

                // Make sure there's no ReadAsync pending
                _ = to.IcePingAsync();
                _ = timeout.IcePingAsync();

                var semaphore = new System.Threading.SemaphoreSlim(0);
                connection.Closed += (sender, args) => semaphore.Release();
                connection.Close(ConnectionClose.Gracefully);
                TestHelper.Assert(semaphore.Wait(500));

                connection2.Closed += (sender, args) => semaphore.Release();
                connection2.Close(ConnectionClose.Gracefully);
                TestHelper.Assert(!semaphore.Wait(500));

                controller.ResumeAdapter();
                timeout.Op(); // Ensure adapter is active.
            }
            output.WriteLine("ok");

            output.Write("testing invocation timeouts with collocated calls... ");
            output.Flush();
            {
                communicator.SetProperty("TimeoutCollocated.AdapterId", "timeoutAdapter");

                ObjectAdapter adapter = communicator.CreateObjectAdapter("TimeoutCollocated");
                adapter.Activate();

                ITimeoutPrx proxy = adapter.AddWithUUID(new Timeout(), ITimeoutPrx.Factory);
                try
                {
                    using var timeoutTokenSource = new CancellationTokenSource(TimeSpan.FromMilliseconds(100));
                    proxy.SleepAsync(500, cancel: timeoutTokenSource.Token).Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ex) when (ex.InnerException is OperationCanceledException)
                {
                }

                adapter.Dispose();
            }
            output.WriteLine("ok");

            controller.Shutdown();
        }
    }
}
