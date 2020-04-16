//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using Ice.timeout.Test;
using Test;

namespace Ice.timeout
{
    public class AllTests
    {
        private static Connection connect(IObjectPrx prx)
        {
            int nRetry = 10;
            while (--nRetry > 0)
            {
                try
                {
                    prx.GetConnection();
                    break;
                }
                catch (ConnectTimeoutException)
                {
                    // Can sporadically occur with slow machines
                }
            }
            return prx.GetConnection();
        }

        public static void allTests(global::Test.TestHelper helper)
        {
            var communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            var controller = IControllerPrx.Parse($"controller:{helper.GetTestEndpoint(1)}", communicator);
            try
            {
                allTestsWithController(helper, controller);
            }
            catch (Exception)
            {
                // Ensure the adapter is not in the holding state when an unexpected exception occurs to prevent
                // the test from hanging on exit in case a connection which disables timeouts is still opened.
                controller.resumeAdapter();
                throw;
            }
        }

        public static void allTestsWithController(TestHelper helper, IControllerPrx controller)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            var timeout = ITimeoutPrx.Parse($"timeout:{helper.GetTestEndpoint(0)}", communicator);
            System.IO.TextWriter output = helper.GetWriter();
            output.Write("testing connect timeout... ");
            output.Flush();
            {
                //
                // Expect ConnectTimeoutException.
                //
                ITimeoutPrx to = timeout.Clone(connectionTimeout: 100);
                controller.holdAdapter(-1);
                try
                {
                    to.op();
                    TestHelper.Assert(false);
                }
                catch (ConnectTimeoutException)
                {
                    // Expected.
                }
                controller.resumeAdapter();
                timeout.op(); // Ensure adapter is active.
            }
            {
                //
                // Expect success.
                //
                ITimeoutPrx to = timeout.Clone(connectionTimeout: -1);
                controller.holdAdapter(100);
                to.op();
            }
            output.WriteLine("ok");

            // The sequence needs to be large enough to fill the write/recv buffers
            byte[] seq = new byte[2000000];

            // TODO: remove or refactor depending on what we decide for connection timeouts
            // output.Write("testing connection timeout... ");
            // output.Flush();
            // {
            //     //
            //     // Expect TimeoutException.
            //     //
            //     var to = timeout.Clone(connectionTimeout: 250);
            //     connect(to);
            //     controller.holdAdapter(-1);
            //     try
            //     {
            //         to.sendData(seq);
            //         test(false);
            //     }
            //     catch (ConnectionTimeoutException)
            //     {
            //         // Expected.
            //     }
            //     controller.resumeAdapter();
            //     timeout.op(); // Ensure adapter is active.
            // }
            // {
            //     //
            //     // Expect success.
            //     //
            //     var to = timeout.Clone(connectionTimeout: 2000);
            //     controller.holdAdapter(100);
            //     try
            //     {
            //         to.sendData(new byte[1000000]);
            //     }
            //     catch (ConnectionTimeoutException)
            //     {
            //         test(false);
            //     }
            // }
            // output.WriteLine("ok");

            output.Write("testing invocation timeout... ");
            output.Flush();
            {
                timeout.IcePing(); // Makes sure a working connection is associated with the proxy
                var connection = timeout.GetConnection();
                var to = timeout.Clone(invocationTimeout: 100);
                TestHelper.Assert(connection == to.GetConnection());
                try
                {
                    to.sleep(1000);
                    TestHelper.Assert(false);
                }
                catch (TimeoutException)
                {
                }
                timeout.IcePing();
                to = timeout.Clone(invocationTimeout: 1000);
                TestHelper.Assert(connection == to.GetConnection());
                try
                {
                    to.sleep(100);
                }
                catch (TimeoutException)
                {
                    TestHelper.Assert(false);
                }
                TestHelper.Assert(connection == to.GetConnection());
            }
            {
                //
                // Expect TimeoutException.
                //
                var to = timeout.Clone(invocationTimeout: 100);
                try
                {
                    to.sleepAsync(1000).Wait();
                }
                catch (AggregateException ex) when (ex.InnerException is TimeoutException)
                {
                }
                timeout.IcePing();
            }
            {
                //
                // Expect success.
                //
                var to = timeout.Clone(invocationTimeout: 1000);
                to.sleepAsync(100).Wait();
            }

           output.WriteLine("ok");

            // TODO: remove or refactor depending on what we decide for connection timeouts
            // output.Write("testing close timeout... ");
            // output.Flush();
            // {
            //     var to = timeout.Clone(connectionTimeout: 250);
            //     var connection = connect(to);
            //     controller.holdAdapter(-1);
            //     connection.Close(Ice.ConnectionClose.GracefullyWithWait);
            //     try
            //     {
            //         _ = connection.GetConnectionInfo(); // getInfo() doesn't throw in the closing state.
            //     }
            //     catch (System.Exception)
            //     {
            //         test(false);
            //     }
            //
            //     while (true)
            //     {
            //         try
            //         {
            //             _ = connection.GetConnectionInfo();
            //             Thread.Sleep(10);
            //         }
            //         catch (ConnectionClosedLocallyException ex)
            //         {
            //             // Expected (graceful closure)
            //             break;
            //         }
            //     }
            //     controller.resumeAdapter();
            //     timeout.op(); // Ensure adapter is active.
            // }
            // output.WriteLine("ok");

            output.Write("testing timeout overrides... ");
            output.Flush();
            // TODO: remove or refactor depending on what we decide for connection timeouts
            // {
            //     //
            //     // Test Ice.Override.Timeout. This property overrides all
            //     // endpoint timeouts.
            //     //
            //     var properties = communicator.GetProperties();
            //     properties["Ice.Override.ConnectTimeout"] = "250";
            //     properties["Ice.Override.Timeout"] = "100";
            //     var comm = helper.initialize(properties);
            //     var to = ITimeoutPrx.Parse(sref, comm);
            //     connect(to);
            //     controller.holdAdapter(-1);
            //     try
            //     {
            //         to.sendData(seq);
            //         test(false);
            //     }
            //     catch (ConnectionTimeoutException)
            //     {
            //         // Expected.
            //     }
            //     controller.resumeAdapter();
            //     timeout.op(); // Ensure adapter is active.

            //     //
            //     // Calling ice_timeout() should have no effect.
            //     //
            //     to = to.Clone(connectionTimeout: 1000);
            //     connect(to);
            //     controller.holdAdapter(-1);
            //     try
            //     {
            //         to.sendData(seq);
            //         test(false);
            //     }
            //     catch (ConnectionTimeoutException)
            //     {
            //         // Expected.
            //     }
            //     controller.resumeAdapter();
            //     timeout.op(); // Ensure adapter is active.
            //     comm.Destroy();
            // }

            {
                //
                // Test Ice.Override.ConnectTimeout.
                //
                var properties = communicator.GetProperties();
                properties["Ice.Override.ConnectTimeout"] = "250";
                var comm = helper.Initialize(properties);
                controller.holdAdapter(-1);
                var to = ITimeoutPrx.Parse($"timeout:{helper.GetTestEndpoint(0)}", comm);
                try
                {
                    to.op();
                    TestHelper.Assert(false);
                }
                catch (ConnectTimeoutException)
                {
                    // Expected.
                }
                controller.resumeAdapter();
                timeout.op(); // Ensure adapter is active.

                //
                // Calling ice_timeout() should have no effect on the connect timeout.
                //
                controller.holdAdapter(-1);
                to = to.Clone(connectionTimeout: 1000);
                try
                {
                    to.op();
                    TestHelper.Assert(false);
                }
                catch (ConnectTimeoutException)
                {
                    // Expected.
                }
                controller.resumeAdapter();
                timeout.op(); // Ensure adapter is active.

                //
                // Verify that timeout set via ice_timeout() is still used for requests.
                //
                // to = to.Clone(connectionTimeout: 250);
                // connect(to);
                // controller.holdAdapter(-1);
                // try
                // {
                //     to.sendData(seq);
                //     test(false);
                // }
                // catch (ConnectionTimeoutException)
                // {
                //     // Expected.
                // }
                // controller.resumeAdapter();
                // timeout.op(); // Ensure adapter is active.
                comm.Destroy();
            }
            // TODO: remove or refactor depending on what we decide for connection timeouts
            // {
            //     //
            //     // Test Ice.Override.CloseTimeout.
            //     //
            //     var properties = communicator.GetProperties();
            //     properties["Ice.Override.CloseTimeout"] = "100";
            //     var comm = helper.initialize(properties);
            //     IObjectPrx.Parse(sref, comm).GetConnection();
            //     controller.holdAdapter(-1);
            //     long begin = System.DateTime.Now.Ticks;
            //     comm.Destroy();
            //     test(((long)new System.TimeSpan(System.DateTime.Now.Ticks - begin).TotalMilliseconds - begin) < 1000);
            //     controller.resumeAdapter();
            // }
            output.WriteLine("ok");

            output.Write("testing invocation timeouts with collocated calls... ");
            output.Flush();
            {
                communicator.SetProperty("TimeoutCollocated.AdapterId", "timeoutAdapter");

                var adapter = communicator.CreateObjectAdapter("TimeoutCollocated");
                adapter.Activate();

                var proxy = adapter.AddWithUUID(new Timeout(), ITimeoutPrx.Factory).Clone(invocationTimeout: 100);
                try
                {
                    proxy.sleep(500);
                    TestHelper.Assert(false);
                }
                catch (TimeoutException)
                {
                }

                try
                {
                    proxy.sleepAsync(500).Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ex) when (ex.InnerException is TimeoutException)
                {
                }
                adapter.Destroy();
            }
            output.WriteLine("ok");

            controller.shutdown();
        }
    }
}
