//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;
using System.Threading;
using Ice.timeout.Test;

namespace Ice
{
    namespace timeout
    {
        public class AllTests : global::Test.AllTests
        {
            private static Connection connect(Ice.IObjectPrx prx)
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
                var controller = ControllerPrx.Parse($"controller:{helper.getTestEndpoint(1)}", helper.communicator());
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

            public static void allTestsWithController(global::Test.TestHelper helper, Test.ControllerPrx controller)
            {
                var communicator = helper.communicator();
                string sref = "timeout:" + helper.getTestEndpoint(0);
                var timeout = TimeoutPrx.Parse(sref, communicator);
                var output = helper.getWriter();
                output.Write("testing connect timeout... ");
                output.Flush();
                {
                    //
                    // Expect ConnectTimeoutException.
                    //
                    var to = timeout.Clone(connectionTimeout: 100);
                    controller.holdAdapter(-1);
                    try
                    {
                        to.op();
                        test(false);
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
                    var to = timeout.Clone(connectionTimeout: -1);
                    controller.holdAdapter(100);
                    try
                    {
                        to.op();
                    }
                    catch (Ice.ConnectTimeoutException)
                    {
                        test(false);
                    }
                }
                output.WriteLine("ok");

                // The sequence needs to be large enough to fill the write/recv buffers
                byte[] seq = new byte[2000000];

                output.Write("testing connection timeout... ");
                output.Flush();
                {
                    //
                    // Expect TimeoutException.
                    //
                    var to = timeout.Clone(connectionTimeout: 250);
                    connect(to);
                    controller.holdAdapter(-1);
                    try
                    {
                        to.sendData(seq);
                        test(false);
                    }
                    catch (TimeoutException)
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
                    var to = timeout.Clone(connectionTimeout: 2000);
                    controller.holdAdapter(100);
                    try
                    {
                        to.sendData(new byte[1000000]);
                    }
                    catch (TimeoutException)
                    {
                        test(false);
                    }
                }
                output.WriteLine("ok");

                output.Write("testing invocation timeout... ");
                output.Flush();
                {
                    var connection = timeout.GetConnection();
                    var to = timeout.Clone(invocationTimeout: 100);
                    test(connection == to.GetConnection());
                    try
                    {
                        to.sleep(1000);
                        test(false);
                    }
                    catch (InvocationTimeoutException)
                    {
                    }
                    timeout.IcePing();
                    to = timeout.Clone(invocationTimeout: 1000);
                    test(connection == to.GetConnection());
                    try
                    {
                        to.sleep(100);
                    }
                    catch (InvocationTimeoutException)
                    {
                        test(false);
                    }
                    test(connection == to.GetConnection());
                }
                {
                    //
                    // Expect InvocationTimeoutException.
                    //
                    var to = timeout.Clone(invocationTimeout: 100);
                    try
                    {
                        to.sleepAsync(1000).Wait();
                    }
                    catch (System.AggregateException ex) when (ex.InnerException is InvocationTimeoutException)
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
                {
                    //
                    // Backward compatible connection timeouts
                    //
                    var to = timeout.Clone(invocationTimeout: -2, connectionTimeout: 250);
                    var con = connect(to);
                    try
                    {
                        to.sleep(750);
                        test(false);
                    }
                    catch (TimeoutException)
                    {
                        try
                        {
                            con.getInfo();
                            test(false);
                        }
                        catch (TimeoutException)
                        {
                            // Connection got closed as well.
                        }
                    }
                    timeout.IcePing();

                    try
                    {
                        con = connect(to);
                        to.sleepAsync(750).Wait();
                        test(false);
                    }
                    catch (System.AggregateException ex) when (ex.InnerException is TimeoutException)
                    {
                        try
                        {
                            con.getInfo();
                            test(false);
                        }
                        catch (TimeoutException)
                        {
                            // Connection got closed as well.
                        }
                    }
                    timeout.IcePing();
                }
                output.WriteLine("ok");

                output.Write("testing close timeout... ");
                output.Flush();
                {
                    var to = timeout.Clone(connectionTimeout: 250);
                    var connection = connect(to);
                    controller.holdAdapter(-1);
                    connection.close(Ice.ConnectionClose.GracefullyWithWait);
                    try
                    {
                        connection.getInfo(); // getInfo() doesn't throw in the closing state.
                    }
                    catch (LocalException)
                    {
                        test(false);
                    }

                    while (true)
                    {
                        try
                        {
                            connection.getInfo();
                            Thread.Sleep(10);
                        }
                        catch (ConnectionManuallyClosedException ex)
                        {
                            // Expected.
                            test(ex.graceful);
                            break;
                        }
                    }
                    controller.resumeAdapter();
                    timeout.op(); // Ensure adapter is active.
                }
                output.WriteLine("ok");

                output.Write("testing timeout overrides... ");
                output.Flush();
                {
                    //
                    // Test Ice.Override.Timeout. This property overrides all
                    // endpoint timeouts.
                    //
                    var initData = new Ice.InitializationData();
                    initData.properties = communicator.getProperties().Clone();
                    initData.properties.setProperty("Ice.Override.ConnectTimeout", "250");
                    initData.properties.setProperty("Ice.Override.Timeout", "100");
                    var comm = helper.initialize(initData);
                    var to = TimeoutPrx.Parse(sref, comm);
                    connect(to);
                    controller.holdAdapter(-1);
                    try
                    {
                        to.sendData(seq);
                        test(false);
                    }
                    catch (TimeoutException)
                    {
                        // Expected.
                    }
                    controller.resumeAdapter();
                    timeout.op(); // Ensure adapter is active.

                    //
                    // Calling ice_timeout() should have no effect.
                    //
                    to = to.Clone(connectionTimeout: 1000);
                    connect(to);
                    controller.holdAdapter(-1);
                    try
                    {
                        to.sendData(seq);
                        test(false);
                    }
                    catch (TimeoutException)
                    {
                        // Expected.
                    }
                    controller.resumeAdapter();
                    timeout.op(); // Ensure adapter is active.
                    comm.destroy();
                }
                {
                    //
                    // Test Ice.Override.ConnectTimeout.
                    //
                    var initData = new InitializationData();
                    initData.properties = communicator.getProperties().Clone();
                    initData.properties.setProperty("Ice.Override.ConnectTimeout", "250");
                    var comm = helper.initialize(initData);
                    controller.holdAdapter(-1);
                    var to = TimeoutPrx.Parse(sref, comm);
                    try
                    {
                        to.op();
                        test(false);
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
                        test(false);
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
                    to = to.Clone(connectionTimeout: 250);
                    connect(to);
                    controller.holdAdapter(-1);
                    try
                    {
                        to.sendData(seq);
                        test(false);
                    }
                    catch (TimeoutException)
                    {
                        // Expected.
                    }
                    controller.resumeAdapter();
                    timeout.op(); // Ensure adapter is active.
                    comm.destroy();
                }
                {
                    //
                    // Test Ice.Override.CloseTimeout.
                    //
                    var initData = new InitializationData();
                    initData.properties = communicator.getProperties().Clone();
                    initData.properties.setProperty("Ice.Override.CloseTimeout", "100");
                    var comm = helper.initialize(initData);
                    IObjectPrx.Parse(sref, comm).GetConnection();
                    controller.holdAdapter(-1);
                    long begin = System.DateTime.Now.Ticks;
                    comm.destroy();
                    test(((long)new System.TimeSpan(System.DateTime.Now.Ticks - begin).TotalMilliseconds - begin) < 1000);
                    controller.resumeAdapter();
                }
                output.WriteLine("ok");

                output.Write("testing invocation timeouts with collocated calls... ");
                output.Flush();
                {
                    communicator.getProperties().setProperty("TimeoutCollocated.AdapterId", "timeoutAdapter");

                    var adapter = communicator.createObjectAdapter("TimeoutCollocated");
                    adapter.Activate();

                    var proxy = adapter.Add(new TimeoutI()).Clone(invocationTimeout: 100);
                    try
                    {
                        proxy.sleep(500);
                        test(false);
                    }
                    catch (InvocationTimeoutException)
                    {
                    }

                    try
                    {
                        proxy.sleepAsync(500).Wait();
                        test(false);
                    }
                    catch (System.AggregateException ex) when (ex.InnerException is InvocationTimeoutException)
                    {
                    }

                    try
                    {
                        proxy.Clone(invocationTimeout: -2).IcePing();
                        proxy.Clone(invocationTimeout: -2).IcePingAsync().Wait();
                    }
                    catch (System.Exception)
                    {
                        test(false);
                    }

                    adapter.Destroy();
                }
                output.WriteLine("ok");

                controller.shutdown();
            }
        }
    }
}
