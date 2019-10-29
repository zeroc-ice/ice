//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;

namespace Ice
{
    namespace operations
    {
        class BatchOnewaysAMI
        {
            private static void test(bool b)
            {
                if (!b)
                {
                    throw new System.Exception();
                }
            }

            private class Callback
            {
                internal Callback()
                {
                    _called = false;
                }

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

            internal static void batchOneways(Test.MyClassPrx p)
            {
                byte[] bs1 = new byte[10 * 1024];

                Test.MyClassPrx batch = Test.MyClassPrxHelper.uncheckedCast(p.ice_batchOneway());
                batch.ice_flushBatchRequestsAsync().Wait();

                batch.ice_flushBatchRequestsAsync(progress: new Progress<bool>(sentSynchronously =>
                    {
                        test(sentSynchronously);
                    })).Wait();

                for (int i = 0; i < 30; ++i)
                {
                    batch.opByteSOnewayAsync(bs1);
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

                    batch1.ice_pingAsync();
                    batch2.ice_pingAsync();
                    batch1.ice_flushBatchRequestsAsync().Wait();
                    batch1.ice_getConnection().close(ConnectionClose.GracefullyWithWait);
                    batch1.ice_pingAsync();
                    batch2.ice_pingAsync();

                    batch1.ice_getConnection();
                    batch2.ice_getConnection();

                    batch1.ice_pingAsync();
                    batch1.ice_getConnection().close(ConnectionClose.GracefullyWithWait);

                    batch1.ice_pingAsync();
                    batch2.ice_pingAsync();
                }

                Identity identity = new Identity();
                identity.name = "invalid";
                ObjectPrx batch3 = batch.ice_identity(identity);
                batch3.ice_pingAsync();
                batch3.ice_flushBatchRequestsAsync().Wait();

                // Make sure that a bogus batch request doesn't cause troubles to other ones.
                batch3.ice_pingAsync();
                batch.ice_pingAsync();
                batch.ice_flushBatchRequestsAsync().Wait();
                batch.ice_pingAsync().Wait();
            }
        }
    }
}
