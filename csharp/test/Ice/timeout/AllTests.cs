// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

namespace Ice.timeout;

public class AllTests : global::Test.AllTests
{
    private static Ice.Connection connect(ObjectPrx prx)
    {
        int nRetry = 10;
        while (--nRetry > 0)
        {
            try
            {
                prx.ice_getConnection();
                break;
            }
            catch (ConnectTimeoutException)
            {
                // Can sporadically occur with slow machines
            }
        }
        return prx.ice_getConnection();
    }

    public static async Task allTests(global::Test.TestHelper helper)
    {
        Test.ControllerPrx controller = Test.ControllerPrxHelper.createProxy(
            helper.communicator(),
            "controller:" + helper.getTestEndpoint(1));

        // Make sure the controller is connected before we proceed.
        _ = connect(controller);

        try
        {
            await allTestsWithController(helper, controller);
        }
        catch (Exception)
        {
            // Ensure the adapter is not in the holding state when an unexpected exception occurs to prevent
            // the test from hanging on exit in case a connection which disables timeouts is still opened.
            controller.resumeAdapter();
            throw;
        }
    }

    public static async Task allTestsWithController(global::Test.TestHelper helper, Test.ControllerPrx controller)
    {
        var communicator = helper.communicator();
        string sref = "timeout:" + helper.getTestEndpoint(0);

        Test.TimeoutPrx timeout = Test.TimeoutPrxHelper.createProxy(communicator, sref);

        var output = helper.getWriter();
        output.Write("testing connect timeout... ");
        output.Flush();
        {
            //
            // Expect ConnectTimeoutException.
            //
            controller.holdAdapter(-1);
            try
            {
                Test.TimeoutPrxHelper.uncheckedCast(timeout.ice_connectionId("connection-1")).op();
                test(false);
            }
            catch (ConnectTimeoutException)
            {
                // Expected.
            }

            if (!Ice.Internal.AssemblyUtil.isMacOS)
            {
                // Workaround for macOS bug
                // See: https://github.com/dotnet/runtime/issues/102663
                try
                {
                    Test.TimeoutPrxHelper.uncheckedCast(timeout.ice_connectionId("connection-2")).op();
                    test(false);
                }
                catch (ConnectTimeoutException)
                {
                    // Expected.
                }
            }
            controller.resumeAdapter();
            Test.TimeoutPrxHelper.uncheckedCast(timeout.ice_connectionId("connection-3")).op(); // Ensure adapter is active.
        }
        {
            //
            // Expect success.
            //
            Properties properties = communicator.getProperties().Clone();
            properties.setProperty("Ice.Connection.Client.ConnectTimeout", "-1");
            using var communicator2 = Util.initialize(new InitializationData { properties = properties });
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.createProxy(communicator2, sref);
            controller.holdAdapter(100);
            try
            {
                to.op();
            }
            catch (ConnectTimeoutException)
            {
                test(false);
            }
        }
        output.WriteLine("ok");

        output.Write("testing invocation timeout... ");
        output.Flush();
        {
            var connection = timeout.ice_getConnection();
            var to = (Test.TimeoutPrx)timeout.ice_invocationTimeout(100);
            test(connection == to.ice_getConnection());
            try
            {
                to.sleep(1000);
                test(false);
            }
            catch (InvocationTimeoutException)
            {
            }
            timeout.ice_ping();
            to = (Test.TimeoutPrx)timeout.ice_invocationTimeout(1000);
            test(connection == to.ice_getConnection());
            try
            {
                to.sleep(100);
            }
            catch (InvocationTimeoutException)
            {
                test(false);
            }
            test(connection == to.ice_getConnection());
        }
        {
            //
            // Expect InvocationTimeoutException.
            //
            var to = (Test.TimeoutPrx)timeout.ice_invocationTimeout(100);
            try
            {
                await to.sleepAsync(1000);
                test(false);
            }
            catch (InvocationTimeoutException)
            {
            }
            timeout.ice_ping();
        }
        {
            //
            // Expect success.
            //
            var to = (Test.TimeoutPrx)timeout.ice_invocationTimeout(1000);
            await to.sleepAsync(100);
        }
        output.WriteLine("ok");

        output.Write("testing close timeout... ");
        output.Flush();
        {
            var connection = connect(timeout);
            controller.holdAdapter(-1);
            Task closeTask = connection.closeAsync(); // initiate closure
            try
            {
                connection.getInfo(); // getInfo() doesn't throw in the closing state.
            }
            catch (LocalException)
            {
                test(false);
            }

            try
            {
                await closeTask; // wait for close to complete
                test(false);
            }
            catch (CloseTimeoutException)
            {
                // Expected.
            }

            try
            {
                connection.getInfo();
                test(false);
            }
            catch (CloseTimeoutException)
            {
                // Expected.
            }

            controller.resumeAdapter();
            timeout.op(); // Ensure adapter is active.
        }
        output.WriteLine("ok");

        output.Write("testing invocation timeouts with collocated calls... ");
        output.Flush();
        {
            communicator.getProperties().setProperty("TimeoutCollocated.AdapterId", "timeoutAdapter");

            var adapter = communicator.createObjectAdapter("TimeoutCollocated");
            adapter.activate();

            Test.TimeoutPrx proxy = Test.TimeoutPrxHelper.uncheckedCast(adapter.addWithUUID(new TimeoutI()));
            proxy = (Test.TimeoutPrx)proxy.ice_invocationTimeout(100);
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
                await proxy.sleepAsync(500);
                test(false);
            }
            catch (InvocationTimeoutException)
            {
            }

            var batchTimeout = (Test.TimeoutPrx)proxy.ice_batchOneway();
            batchTimeout.ice_ping();
            batchTimeout.ice_ping();
            batchTimeout.ice_ping();

            _ = ((Test.TimeoutPrx)proxy.ice_invocationTimeout(-1)).sleepAsync(500); // Keep the server thread pool busy.
            try
            {
                await batchTimeout.ice_flushBatchRequestsAsync();
                test(false);
            }
            catch (InvocationTimeoutException)
            {
            }

            adapter.destroy();
        }
        output.WriteLine("ok");

        controller.shutdown();
    }
}
