// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.maxDispatches;

internal class AllTests : global::Test.AllTests
{
    internal static async Task allTests(global::Test.TestHelper helper)
    {
        Communicator communicator = helper.communicator();
        string proxyString = $"test: {helper.getTestEndpoint()}";
        Test.TestIntfPrx p = Test.TestIntfPrxHelper.createProxy(communicator, proxyString);

        string responderProxyString = $"responder: {helper.getTestEndpoint(1)}";
        Test.ResponderPrx responder = Test.ResponderPrxHelper.createProxy(communicator, responderProxyString);

        string proxyStringMax10 = $"test: {helper.getTestEndpoint(2)}";
        Test.TestIntfPrx pMax10 = Test.TestIntfPrxHelper.createProxy(communicator, proxyStringMax10);

        string proxyStringMax1 = $"test: {helper.getTestEndpoint(3)}";
        Test.TestIntfPrx pMax1 = Test.TestIntfPrxHelper.createProxy(communicator, proxyStringMax1);

        string proxyStringSerialize = $"test: {helper.getTestEndpoint(4)}";
        Test.TestIntfPrx pSerialize = Test.TestIntfPrxHelper.createProxy(communicator, proxyStringSerialize);

        await testMaxDispatches(p, responder, 100, helper.getWriter());
        await testMaxDispatches(pMax10, responder, 10, helper.getWriter());
        await testMaxDispatches(pMax1, responder, 1, helper.getWriter());

        // Serialize does not limit dispatches with "true" AMD.
        await testMaxDispatches(pSerialize, responder, 100, helper.getWriter());

        await p.shutdownAsync();
    }

    // Verifies max dispatches is implemented correctly.
    private static async Task testMaxDispatches(
        Test.TestIntfPrx p,
        Test.ResponderPrx responder,
        int maxCount,
        TextWriter output)
    {
        output.Write($"testing max dispatches max = {maxCount}... ");
        output.Flush();

        // Make sure we start fresh
        await responder.stopAsync();
        test(await responder.pendingResponseCountAsync() == 0);
        _ = await p.resetMaxConcurrentDispatchesAsync();

        var taskList = new List<Task>();

        for (int i = 0; i < maxCount + 20; ++i)
        {
            taskList.Add(p.opAsync());
        }

        // Wait until the responder gets at least maxCount responses.
        while (await responder.pendingResponseCountAsync() < maxCount)
        {
            await Task.Delay(TimeSpan.FromMilliseconds(10));
        }

        await responder.startAsync();
        await Task.WhenAll(taskList);

        int maxConcurrentDispatches = await p.resetMaxConcurrentDispatchesAsync();
        test(maxConcurrentDispatches == maxCount);
        output.WriteLine("ok");
    }
}
