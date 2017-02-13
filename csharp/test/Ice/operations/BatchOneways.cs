// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

class BatchOneways
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }

    class BatchRequestInterceptorI : Ice.BatchRequestInterceptor
    {
        public void enqueue(Ice.BatchRequest request, int count, int size)
        {
            test(request.getOperation().Equals("opByteSOneway") || request.getOperation().Equals("ice_ping"));
            test(request.getProxy().ice_isBatchOneway());

            if(count > 0)
            {
                test(_lastRequestSize + _size == size);
            }
            _count = count;
            _size = size;

            if(_size + request.getSize() > 25000)
            {
                request.getProxy().begin_ice_flushBatchRequests();
                _size = 18; // header
            }

            if(_enabled)
            {
                _lastRequestSize = request.getSize();
                ++_count;
                request.enqueue();
            }
        }

        public void setEnqueue(bool enabled)
        {
            _enabled = enabled;
        }

        public int count()
        {
            return _count;
        }

        private bool _enabled;
        private int _count;
        private int _size;
        private int _lastRequestSize;
    };

    internal static void batchOneways(Test.MyClassPrx p)
    {
        byte[] bs1 = new byte[10  * 1024];

        Test.MyClassPrx batch = Test.MyClassPrxHelper.uncheckedCast(p.ice_batchOneway());
        batch.ice_flushBatchRequests(); // Empty flush

        p.opByteSOnewayCallCount(); // Reset the call count

        for(int i = 0 ; i < 30 ; ++i)
        {
            try
            {
                batch.opByteSOneway(bs1);
                test(true);
            }
            catch(Ice.MemoryLimitException)
            {
                test(false);
            }
        }

        int count = 0;
        while(count < 27) // 3 * 9 requests auto-flushed.
        {
            count += p.opByteSOnewayCallCount();
            System.Threading.Thread.Sleep(10);
        }

        if(batch.ice_getConnection() != null)
        {
            Test.MyClassPrx batch1 = Test.MyClassPrxHelper.uncheckedCast(p.ice_batchOneway());
            Test.MyClassPrx batch2 = Test.MyClassPrxHelper.uncheckedCast(p.ice_batchOneway());

            batch1.ice_ping();
            batch2.ice_ping();
            batch1.ice_flushBatchRequests();
            batch1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
            batch1.ice_ping();
            batch2.ice_ping();

            batch1.ice_getConnection();
            batch2.ice_getConnection();

            batch1.ice_ping();
            batch1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
            batch1.ice_ping();
            batch2.ice_ping();
        }

        Ice.Identity identity = new Ice.Identity();
        identity.name = "invalid";
        Ice.ObjectPrx batch3 = batch.ice_identity(identity);
        batch3.ice_ping();
        batch3.ice_flushBatchRequests();

        // Make sure that a bogus batch request doesn't cause troubles to other ones.
        batch3.ice_ping();
        batch.ice_ping();
        batch.ice_flushBatchRequests();
        batch.ice_ping();

        if(batch.ice_getConnection() != null)
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = p.ice_getCommunicator().getProperties().ice_clone_();
            BatchRequestInterceptorI interceptor = new BatchRequestInterceptorI();
            initData.batchRequestInterceptor = interceptor;
            Ice.Communicator ic = Ice.Util.initialize(initData);

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
        if(p.ice_getConnection() != null &&
           p.ice_getCommunicator().getProperties().getProperty("Ice.Override.Compress").Equals(""))
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
