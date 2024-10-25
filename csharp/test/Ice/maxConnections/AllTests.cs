// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.maxConnections;

internal class AllTests : global::Test.AllTests
{
    internal static async Task allTests(global::Test.TestHelper helper)
    {
        Communicator communicator = helper.communicator();
        string proxyString = $"test: {helper.getTestEndpoint()}";
        Test.TestIntfPrx p = Test.TestIntfPrxHelper.createProxy(communicator, proxyString);

        string proxyStringMax10 = $"test: {helper.getTestEndpoint(1)}";
        Test.TestIntfPrx pMax10 = Test.TestIntfPrxHelper.createProxy(communicator, proxyStringMax10);

        // When the transport is WS or WSS, we need to wait a little bit: the server closes the connection after it
        // gets a transport frame from the client.
        Func<Task>? postCloseDelay = null;
        if (helper.getTestProtocol().StartsWith("ws"))
        {
            postCloseDelay = () => Task.Delay(TimeSpan.FromMilliseconds(50));
        }

        await testCreateConnections(p, 100, helper.getWriter(), postCloseDelay);
        await testCreateConnectionsWithMax(pMax10, 10, helper.getWriter(), postCloseDelay);
        await testCreateConnectionsWithMaxAndRecovery(pMax10, 10, helper.getWriter(), postCloseDelay);

        await p.shutdownAsync();
    }

    // Verify that we can create connectionCount connections and send a ping on each connection.
    private static async Task testCreateConnections(
        Test.TestIntfPrx p,
        int connectionCount,
        TextWriter output,
        Func<Task>? postCloseDelay)
    {
        output.Write($"testing the creation of {connectionCount} connections... ");
        output.Flush();

        var connectionList = new List<Ice.Connection>();
        for (int i = 0; i < connectionCount; i++)
        {
            p = Test.TestIntfPrxHelper.uncheckedCast(p.ice_connectionId($"connection-{i}"));
            await p.ice_pingAsync();
            connectionList.Add(p.ice_getCachedConnection()!);
        }

        // Close all connections
        await Task.WhenAll(connectionList.Select(c => c.closeAsync()));

        if (postCloseDelay is not null)
        {
            await postCloseDelay();
        }
        output.WriteLine("ok");
    }

    // Verify that we can create max connections but not more.
    private static async Task testCreateConnectionsWithMax(
        Test.TestIntfPrx p,
        int max,
        TextWriter output,
        Func<Task>? postCloseDelay)
    {
        output.Write($"testing the creation of {max} connections with connection lost at {max + 1}... ");
        output.Flush();

        var connectionList = new List<Ice.Connection>();
        for (int i = 0; i < max; i++)
        {
            p = Test.TestIntfPrxHelper.uncheckedCast(p.ice_connectionId($"connection-{i}"));
            await p.ice_pingAsync();
            connectionList.Add(p.ice_getCachedConnection()!);
        }

        p = Test.TestIntfPrxHelper.uncheckedCast(p.ice_connectionId($"connection-{max}"));
        try
        {
            await p.ice_pingAsync();
            test(false);
        }
        catch (Ice.ConnectionLostException)
        {
            // expected, the server aborts the connection when MaxConnections is reached
        }

        // Close all connections
        await Task.WhenAll(connectionList.Select(c => c.closeAsync()));

        if (postCloseDelay is not null)
        {
            await postCloseDelay();
        }
        output.WriteLine("ok");
    }

    // Verify that we can create max connections, then connection lost, then recover.
    private static async Task testCreateConnectionsWithMaxAndRecovery(
        Test.TestIntfPrx p,
        int max,
        TextWriter output,
        Func<Task>? postCloseDelay)
    {
        output.Write($"testing the creation of {max} connections with connection lost at {max + 1} then recovery... ");
        output.Flush();

        var connectionList = new List<Ice.Connection>();
        for (int i = 0; i < max; i++)
        {
            p = Test.TestIntfPrxHelper.uncheckedCast(p.ice_connectionId($"connection-{i}"));
            await p.ice_pingAsync();
            connectionList.Add(p.ice_getCachedConnection()!);
        }

        p = Test.TestIntfPrxHelper.uncheckedCast(p.ice_connectionId($"connection-{max}"));
        try
        {
            await p.ice_pingAsync();
            test(false);
        }
        catch (Ice.ConnectionLostException)
        {
            // expected
        }

        // Close one connection
        await connectionList[0].closeAsync();
        connectionList.RemoveAt(0);

        if (postCloseDelay is not null)
        {
            await postCloseDelay();
        }
        else
        {
            // We need to wait a tiny bit to let the server remove the connection from its incoming connection
            // factory.
            await Task.Delay(TimeSpan.FromMilliseconds(10));
        }

        // Try again
        await p.ice_pingAsync();
        connectionList.Add(p.ice_getCachedConnection()!);

        // Close all connections
        await Task.WhenAll(connectionList.Select(c => c.closeAsync()));

        if (postCloseDelay is not null)
        {
            await postCloseDelay();
        }
        output.WriteLine("ok");
    }
}
