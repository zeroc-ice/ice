// Copyright (c) ZeroC, Inc.

namespace Ice.hold;

public class AllTests : global::Test.AllTests
{
    public static async Task allTests(global::Test.TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        var output = helper.getWriter();
        output.Write("testing stringToProxy... ");
        output.Flush();
        string @ref = "hold:" + helper.getTestEndpoint(0);
        Ice.ObjectPrx @base = communicator.stringToProxy(@ref);
        test(@base != null);
        string refSerialized = "hold:" + helper.getTestEndpoint(1);
        Ice.ObjectPrx baseSerialized = communicator.stringToProxy(refSerialized);
        test(baseSerialized != null);
        output.WriteLine("ok");

        output.Write("testing checked cast... ");
        output.Flush();
        var hold = Test.HoldPrxHelper.checkedCast(@base);
        var holdOneway = Test.HoldPrxHelper.uncheckedCast(@base.ice_oneway());
        test(hold != null);
        test(hold.Equals(@base));
        var holdSerialized = Test.HoldPrxHelper.checkedCast(baseSerialized);
        var holdSerializedOneway = Test.HoldPrxHelper.uncheckedCast(baseSerialized.ice_oneway());
        test(holdSerialized != null);
        test(holdSerialized.Equals(baseSerialized));
        output.WriteLine("ok");

        output.Write("changing state between active and hold rapidly... ");
        output.Flush();
        for (int i = 0; i < 100; ++i)
        {
            hold.putOnHold(0);
        }
        for (int i = 0; i < 100; ++i)
        {
            holdOneway.putOnHold(0);
        }
        for (int i = 0; i < 100; ++i)
        {
            holdSerialized.putOnHold(0);
        }
        for (int i = 0; i < 100; ++i)
        {
            holdSerializedOneway.putOnHold(0);
        }
        output.WriteLine("ok");

        output.Write("testing without serialize mode... ");
        output.Flush();
        var rand = new Random();
        {
            var outOfOrderTcs = new TaskCompletionSource();
            TaskCompletionSource sentTcs = null;
            int value = 0;
            while (!outOfOrderTcs.Task.IsCompleted)
            {
                sentTcs = new TaskCompletionSource();
                int expected = value;
                var t = hold.setAsync(
                    ++value, value < 500 ? rand.Next(5) : 0,
                    progress: new Progress<bool>(value => sentTcs.TrySetResult()));
                _ = Task.Run(
                    async () =>
                    {
                        var response = await t;
                        if (response != expected)
                        {
                            outOfOrderTcs.TrySetResult();
                        }
                    });
                if (value % 100 == 0)
                {
                    await sentTcs.Task;
                }

                if (value > 100000)
                {
                    // Don't continue, it's possible that out-of-order dispatch doesn't occur
                    // after 100000 iterations and we don't want the test to last for too long
                    // when this occurs.
                    break;
                }
            }
            test(value > 100000 || outOfOrderTcs.Task.IsCompleted);
            await sentTcs.Task;
        }
        output.WriteLine("ok");

        output.Write("testing with serialize mode... ");
        output.Flush();
        {
            var outOfOrderTcs = new TaskCompletionSource();
            TaskCompletionSource sentTcs = null;
            int value = 0;
            while (value < 3000 && !outOfOrderTcs.Task.IsCompleted)
            {
                sentTcs = new TaskCompletionSource();
                int expected = value;
                _ = holdSerialized.setAsync(
                    ++value,
                    0,
                    progress: new Progress<bool>(value => sentTcs.TrySetResult()));
                if (value % 100 == 0)
                {
                    await sentTcs.Task;
                }
            }
            await sentTcs.Task;
            test(!outOfOrderTcs.Task.IsCompleted);

            for (int i = 0; i < 10000; ++i)
            {
                holdSerializedOneway.setOneway(value + 1, value);
                ++value;
                if ((i % 100) == 0)
                {
                    holdSerializedOneway.putOnHold(1);
                }
            }
        }
        output.WriteLine("ok");

        output.Write("testing serialization... ");
        output.Flush();
        {
            int value = 0;
            holdSerialized.set(value, 0);
            var outOfOrderTcs = new TaskCompletionSource();
            TaskCompletionSource sentTcs = null;
            Task result = null;

            // We use the same proxy for all oneway calls.
            holdSerializedOneway = (Test.HoldPrx)holdSerialized.ice_oneway();

            for (int i = 0; i < 10000; ++i)
            {
                sentTcs = new TaskCompletionSource();
                result = holdSerializedOneway.setOnewayAsync(
                    value + 1,
                    value,
                    progress: new Progress<bool>(value => sentTcs.TrySetResult()));
                ++value;
                if ((i % 100) == 0)
                {
                    await sentTcs.Task;
                    holdSerialized.ice_ping(); // Ensure everything's dispatched.
                    await holdSerialized.ice_getConnection().closeAsync();
                }
            }
            await result;
        }
        output.WriteLine("ok");

        output.Write("testing waitForHold... ");
        output.Flush();
        {
            hold.waitForHold();
            hold.waitForHold();
            for (int i = 0; i < 1000; ++i)
            {
                holdOneway.ice_ping();
                if ((i % 20) == 0)
                {
                    hold.putOnHold(0);
                }
            }
            hold.putOnHold(-1);
            hold.ice_ping();
            hold.putOnHold(-1);
            hold.ice_ping();
        }
        output.WriteLine("ok");

        output.Write("changing state to hold and shutting down server... ");
        output.Flush();
        hold.shutdown();
        output.WriteLine("ok");
    }
}
