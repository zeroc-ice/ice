// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

namespace Ice.operations;

internal class BatchOnewaysAMI
{
    private static void test(bool b) => global::Test.TestHelper.test(b);

    private class Callback
    {
        internal Callback() => _called = false;

        public void check()
        {
            lock (this)
            {
                while (!_called)
                {
                    System.Threading.Monitor.Wait(this);
                }

                _called = false;
            }
        }

        public void called()
        {
            lock (this)
            {
                Debug.Assert(!_called);
                _called = true;
                System.Threading.Monitor.Pulse(this);
            }
        }

        private bool _called;
    }

    internal static async Task batchOneways(Test.MyClassPrx p)
    {
        byte[] bs1 = new byte[10 * 1024];

        Test.MyClassPrx batch = Test.MyClassPrxHelper.uncheckedCast(p.ice_batchOneway());
        for (int i = 0; i < 30; ++i)
        {
            await batch.opByteSOnewayAsync(bs1);
        }

        int count = 0;
        while (count < 27) // 3 * 9 requests auto-flushed.
        {
            count += p.opByteSOnewayCallCount();
            System.Threading.Thread.Sleep(10);
        }

        if (batch.ice_getConnection() != null)
        {
            Test.MyClassPrx batch1 = Test.MyClassPrxHelper.uncheckedCast(p.ice_batchOneway());
            Test.MyClassPrx batch2 = Test.MyClassPrxHelper.uncheckedCast(p.ice_batchOneway());

            await batch1.ice_pingAsync();
            await batch2.ice_pingAsync();
            await batch1.ice_flushBatchRequestsAsync();
            await batch1.ice_getConnection().closeAsync();
            await batch1.ice_pingAsync();
            await batch2.ice_pingAsync();

            batch1.ice_getConnection();
            batch2.ice_getConnection();

            _ = batch1.ice_pingAsync();
            await batch1.ice_getConnection().closeAsync();

            _ = batch1.ice_pingAsync();
            _ = batch2.ice_pingAsync();
        }

        var identity = new Ice.Identity("invalid", "");
        Ice.ObjectPrx batch3 = batch.ice_identity(identity);
        await batch3.ice_pingAsync();
        await batch3.ice_flushBatchRequestsAsync();

        // Make sure that a bogus batch request doesn't cause troubles to other ones.
        _ = batch3.ice_pingAsync();
        _ = batch.ice_pingAsync();
        await batch.ice_flushBatchRequestsAsync();
        _ = batch.ice_pingAsync();
    }
}
