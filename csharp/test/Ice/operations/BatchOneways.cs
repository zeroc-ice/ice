// Copyright (c) ZeroC, Inc.

namespace Ice.operations
{


        internal class BatchOneways
        {
            private static void test(bool b) => global::Test.TestHelper.test(b);

            private class BatchRequestInterceptorI
            {
                public void enqueue(Ice.BatchRequest request, int count, int size)
                {
                    test(request.getOperation() == "opByteSOneway" || request.getOperation() == "ice_ping");
                    test(request.getProxy().ice_isBatchOneway());

                    if (count > 0)
                    {
                        test(_lastRequestSize + _size == size);
                    }
                    _count = count;
                    _size = size;

                    if (_size + request.getSize() > 25000)
                    {
                        _ = request.getProxy().ice_flushBatchRequestsAsync();
                        _size = 18; // header
                    }

                    if (_enabled)
                    {
                        _lastRequestSize = request.getSize();
                        ++_count;
                        request.enqueue();
                    }
                }

                public void setEnqueue(bool enabled) => _enabled = enabled;

                public int count() => _count;

                private bool _enabled;
                private int _count;
                private int _size;
                private int _lastRequestSize;
            }

            internal static void batchOneways(global::Test.TestHelper helper, Test.MyClassPrx p)
            {
                byte[] bs1 = new byte[10 * 1024];

                Test.MyClassPrx batch = Test.MyClassPrxHelper.uncheckedCast(p.ice_batchOneway());
                batch.ice_flushBatchRequests(); // Empty flush

                p.opByteSOnewayCallCount(); // Reset the call count

                for (int i = 0; i < 30; ++i)
                {
                    try
                    {
                        batch.opByteSOneway(bs1);
                    }
                    catch (Ice.MarshalException)
                    {
                        test(false);
                    }
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

                    batch1.ice_ping();
                    batch2.ice_ping();
                    batch1.ice_flushBatchRequests();
                    _ = batch1.ice_getConnection().closeAsync();
                    batch1.ice_ping();
                    batch2.ice_ping();

                    batch1.ice_getConnection();
                    batch2.ice_getConnection();

                    batch1.ice_ping();
                    _ = batch1.ice_getConnection().closeAsync();
                    batch1.ice_ping();
                    batch2.ice_ping();
                }

                var identity = new Identity("invalid", "");
                var batch3 = batch.ice_identity(identity);
                batch3.ice_ping();
                batch3.ice_flushBatchRequests();

                // Make sure that a bogus batch request doesn't cause troubles to other ones.
                batch3.ice_ping();
                batch.ice_ping();
                batch.ice_flushBatchRequests();
                batch.ice_ping();

                if (batch.ice_getConnection() != null)
                {
                    var initData = new InitializationData();
                    initData.properties = p.ice_getCommunicator().getProperties().Clone();
                    var interceptor = new BatchRequestInterceptorI();
                    initData.batchRequestInterceptor = interceptor.enqueue;
                    Communicator ic = helper.initialize(initData);

                    batch = Test.MyClassPrxHelper.uncheckedCast(ic.stringToProxy(p.ToString()).ice_batchOneway());

                    test(interceptor.count() == 0);
                    batch.ice_ping();
                    batch.ice_ping();
                    batch.ice_ping();
                    test(interceptor.count() == 0);

                    interceptor.setEnqueue(true);
                    batch.ice_ping();
                    batch.ice_ping();
                    batch.ice_ping();
                    test(interceptor.count() == 3);

                    batch.ice_flushBatchRequests();
                    batch.ice_ping();
                    test(interceptor.count() == 1);

                    batch.opByteSOneway(bs1);
                    test(interceptor.count() == 2);
                    batch.opByteSOneway(bs1);
                    test(interceptor.count() == 3);

                    batch.opByteSOneway(bs1); // This should trigger the flush
                    batch.ice_ping();
                    test(interceptor.count() == 2);

                    ic.destroy();
                }

                p.ice_ping();

                bool supportsCompress = true;
                try
                {
                    supportsCompress = p.supportsCompress();
                }
                catch (Ice.OperationNotExistException)
                {
                }

                if (supportsCompress && p.ice_getConnection() != null &&
                   p.ice_getCommunicator().getProperties().getIceProperty("Ice.Override.Compress").Length == 0)
                {
                    Ice.ObjectPrx prx = p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway();

                    Test.MyClassPrx batchC1 = Test.MyClassPrxHelper.uncheckedCast(prx.ice_compress(false));
                    Test.MyClassPrx batchC2 = Test.MyClassPrxHelper.uncheckedCast(prx.ice_compress(true));
                    Test.MyClassPrx batchC3 = Test.MyClassPrxHelper.uncheckedCast(prx.ice_identity(identity));

                    batchC1.opByteSOneway(bs1);
                    batchC1.opByteSOneway(bs1);
                    batchC1.opByteSOneway(bs1);
                    batchC1.ice_getConnection().flushBatchRequests(Ice.CompressBatch.Yes);

                    batchC2.opByteSOneway(bs1);
                    batchC2.opByteSOneway(bs1);
                    batchC2.opByteSOneway(bs1);
                    batchC1.ice_getConnection().flushBatchRequests(Ice.CompressBatch.No);

                    batchC1.opByteSOneway(bs1);
                    batchC1.opByteSOneway(bs1);
                    batchC1.opByteSOneway(bs1);
                    batchC1.ice_getConnection().flushBatchRequests(Ice.CompressBatch.BasedOnProxy);

                    batchC1.opByteSOneway(bs1);
                    batchC2.opByteSOneway(bs1);
                    batchC1.opByteSOneway(bs1);
                    batchC1.ice_getConnection().flushBatchRequests(Ice.CompressBatch.BasedOnProxy);

                    batchC1.opByteSOneway(bs1);
                    batchC3.opByteSOneway(bs1);
                    batchC1.opByteSOneway(bs1);
                    batchC1.ice_getConnection().flushBatchRequests(Ice.CompressBatch.BasedOnProxy);
                }
            }
        }
    }

