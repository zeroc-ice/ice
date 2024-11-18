// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.idleTimeout;

internal class AllTests : global::Test.AllTests
{
    internal static async Task allTests(global::Test.TestHelper helper)
    {
        Communicator communicator = helper.communicator();
        string proxyString = $"test: {helper.getTestEndpoint()}";
        Test.TestIntfPrx p = Test.TestIntfPrxHelper.createProxy(communicator, proxyString);

        string proxyStringDefaultMax = $"test: {helper.getTestEndpoint(1)}";
        string proxyString3s = $"test: {helper.getTestEndpoint(2)}";
        string proxyStringNoIdleTimeout = $"test: {helper.getTestEndpoint(3)}";

        await testIdleCheckDoesNotAbortBackPressuredConnection(p, helper.getWriter());
        await testConnectionAbortedByIdleCheck(proxyStringDefaultMax, communicator.getProperties(), helper.getWriter());
        await testEnableDisableIdleCheck(true, proxyString3s, communicator.getProperties(), helper.getWriter());
        await testEnableDisableIdleCheck(false, proxyString3s, communicator.getProperties(), helper.getWriter());
        await testNoIdleTimeout(proxyStringNoIdleTimeout, communicator.getProperties(), helper.getWriter());

        await p.shutdownAsync();
    }

    // The client and server have the same idle timeout (1s) and both side enable the idle check (the default)
    // We verify that the server's idle check does not abort the connection as long as this connection receives
    // heartbeats, even when the heartbeats are not read off the connection in a timely manner.
    // To verify this situation, we use an OA with a MaxDispatches = 1 to back-pressure the connection.
    private static async Task testIdleCheckDoesNotAbortBackPressuredConnection(
        Test.TestIntfPrx p,
        TextWriter output)
    {
        output.Write("testing that the idle check does not abort a back-pressured connection... ");
        output.Flush();

        // Establish connection.
        await p.ice_pingAsync();

        await p.sleepAsync(2000); // the implementation in the server sleeps for 2,000ms

        // close connection
        await p.ice_getConnection()!.closeAsync();
        output.WriteLine("ok");
    }

    // We verify that the idle check aborts the connection when the connection (here server connection) remains idle for
    // longer than idle timeout. Here, the server has an idle timeout of 1s and the default max dispatches. We
    // intentionally misconfigure the client with an idle timeout of 3s to send heartbeats every 1.5s, which is too long
    // to prevent the server from aborting the connection.
    private static async Task testConnectionAbortedByIdleCheck(
        string proxyString,
        Properties properties,
        TextWriter output)
    {
        output.Write("testing that the idle check aborts a connection that does not receive anything for 1s... ");
        output.Flush();

        // Create a new communicator with the desired properties.
        properties = properties.Clone();
        properties.setProperty("Ice.Connection.Client.IdleTimeout", "3");
        properties.setProperty("Ice.Warn.Connections", "0");
        Communicator communicator = Util.initialize(new InitializationData { properties = properties });
        Test.TestIntfPrx p = Test.TestIntfPrxHelper.createProxy(communicator, proxyString);

        // Establish connection.
        Connection? connection = p.ice_getConnection();
        test(connection is not null);

        // The idle check on the server side aborts the connection because it doesn't get a heartbeat in a timely fashion.
        try
        {
            await p.sleepAsync(2000); // the implementation in the server sleeps for 2,000ms
            test(false);              // we expect the server to abort the connection after about 1 second.
        }
        catch (ConnectionLostException)
        {
            // Expected
        }
        output.WriteLine("ok");
    }

    // Verifies the behavior with the idle check enabled or disabled when the client and the server have mismatched idle
    // timeouts (here: 3s on the server side and 1s on the client side).
    private static async Task testEnableDisableIdleCheck(
        bool enabled,
        string proxyString,
        Properties properties,
        TextWriter output)
    {
        string enabledString = enabled ? "enabled" : "disabled";
        output.Write($"testing connection with idle check {enabledString}... ");
        output.Flush();

        // Create a new communicator with the desired properties.
        properties = properties.Clone();
        properties.setProperty("Ice.Connection.Client.IdleTimeout", "1");
        properties.setProperty("Ice.Connection.Client.EnableIdleCheck", enabled ? "1" : "0");
        properties.setProperty("Ice.Warn.Connections", "0");
        Communicator communicator = Util.initialize(new InitializationData { properties = properties });
        Test.TestIntfPrx p = Test.TestIntfPrxHelper.createProxy(communicator, proxyString);

        Connection? connection = await p.ice_getConnectionAsync();
        test(connection is not null);

        try
        {
            await p.sleepAsync(2000); // the implementation in the server sleeps for 2,000ms
            test(!enabled);
        }
        catch (ConnectionAbortedException)
        {
            test(enabled);
        }
        output.WriteLine("ok");
    }

    // Verifies the idle check is disabled when the idle timeout is set to 0.
    private static async Task testNoIdleTimeout(string proxyString, Properties properties, TextWriter output)
    {
        output.Write("testing connection with idle timeout set to 0... ");
        output.Flush();

        // Create a new communicator with the desired properties.
        properties = properties.Clone();
        properties.setProperty("Ice.Connection.Client.IdleTimeout", "0");
        Communicator communicator = Util.initialize(new InitializationData { properties = properties });
        Test.TestIntfPrx p = Test.TestIntfPrxHelper.createProxy(communicator, proxyString);

        Connection? connection = await p.ice_getConnectionAsync();
        test(connection is not null);

        await p.sleepAsync(2000); // the implementation in the server sleeps for 2,000ms
        await connection!.closeAsync();
        output.WriteLine("ok");
    }
}
