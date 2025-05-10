// Copyright (c) ZeroC, Inc.

package test.Ice.operations;

import com.zeroc.Ice.BatchRequest;
import com.zeroc.Ice.BatchRequestInterceptor;
import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.CompressBatch;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.MarshalException;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.OperationNotExistException;
import com.zeroc.Ice.Properties;

import test.Ice.operations.Test.MyClassPrx;
import test.TestHelper;

import java.io.PrintWriter;

class BatchOneways {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    static class BatchRequestInterceptorI implements BatchRequestInterceptor {
        public void enqueue(BatchRequest request, int count, int size) {
            test(
                "opByteSOneway".equals(request.getOperation())
                    || "ice_ping".equals(request.getOperation()));
            test(request.getProxy().ice_isBatchOneway());

            if (count > 0) {
                test(_lastRequestSize + _size == size);
            }
            _count = count;
            _size = size;

            if (_size + request.getSize() > 25000) {
                request.getProxy().ice_flushBatchRequestsAsync();
                _size = 18; // header
            }

            if (_enabled) {
                _lastRequestSize = request.getSize();
                ++_count;
                request.enqueue();
            }
        }

        public void setEnqueue(boolean enabled) {
            _enabled = enabled;
        }

        public int count() {
            return _count;
        }

        private boolean _enabled;
        private int _count;
        private int _size;
        private int _lastRequestSize;
    }

    static void batchOneways(TestHelper helper, MyClassPrx p, PrintWriter out) {
        final Communicator communicator = helper.communicator();
        final Properties properties = communicator.getProperties();
        final byte[] bs1 = new byte[10 * 1024];

        MyClassPrx batch = p.ice_batchOneway();
        batch.ice_flushBatchRequests(); // Empty flush
        if (batch.ice_getConnection() != null) {
            batch.ice_getConnection().flushBatchRequests(CompressBatch.BasedOnProxy);
        }
        communicator.flushBatchRequests(CompressBatch.BasedOnProxy);

        p.opByteSOnewayCallCount(); // Reset the call count

        for (int i = 0; i < 30; i++) {
            try {
                batch.opByteSOneway(bs1);
            } catch (MarshalException ex) {
                test(false);
            }
        }

        int count = 0;
        while (count < 27) // 3 * 9 requests auto-flushed.
            {
                count += p.opByteSOnewayCallCount();
                try {
                    Thread.sleep(10);
                } catch (InterruptedException ex) {}
            }

        final boolean bluetooth =
            properties.getIceProperty("Ice.Default.Protocol").indexOf("bt") == 0;
        if (batch.ice_getConnection() != null && !bluetooth) {
            MyClassPrx batch1 = p.ice_batchOneway();
            MyClassPrx batch2 = p.ice_batchOneway();

            batch1.ice_ping();
            batch2.ice_ping();
            batch1.ice_flushBatchRequests();
            batch1.ice_getConnection().close();
            batch1.ice_ping();
            batch2.ice_ping();

            batch1.ice_getConnection();
            batch2.ice_getConnection();

            batch1.ice_ping();
            batch1.ice_getConnection().close();
            batch1.ice_ping();
            batch2.ice_ping();
        }

        Identity identity = new Identity();
        identity.name = "invalid";
        ObjectPrx batch3 = batch.ice_identity(identity);
        batch3.ice_ping();
        batch3.ice_flushBatchRequests();

        // Make sure that a bogus batch request doesn't cause troubles to other ones.
        batch3.ice_ping();
        batch.ice_ping();
        batch.ice_flushBatchRequests();
        batch.ice_ping();

        if (batch.ice_getConnection() != null && !bluetooth) {
            InitializationData initData = new InitializationData();
            initData.properties = properties._clone();
            BatchRequestInterceptorI interceptor = new BatchRequestInterceptorI();
            initData.batchRequestInterceptor = interceptor;
            try (Communicator ic = helper.initialize(initData)) {
                batch = MyClassPrx.createProxy(ic, p.toString()).ice_batchOneway();

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
            }
        }

        boolean supportsCompress = true;
        try {
            supportsCompress = p.supportsCompress();
        } catch (OperationNotExistException ex) {}

        p.ice_ping();
        if (supportsCompress
            && p.ice_getConnection() != null
            && properties.getIceProperty("Ice.Override.Compress").isEmpty()) {
            ObjectPrx prx =
                p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway();

            final MyClassPrx batchC1 = MyClassPrx.uncheckedCast(prx.ice_compress(false));
            final MyClassPrx batchC2 = MyClassPrx.uncheckedCast(prx.ice_compress(true));
            final MyClassPrx batchC3 = MyClassPrx.uncheckedCast(prx.ice_identity(identity));

            batchC1.opByteSOneway(bs1);
            batchC1.opByteSOneway(bs1);
            batchC1.opByteSOneway(bs1);
            batchC1.ice_getConnection().flushBatchRequests(CompressBatch.Yes);

            batchC2.opByteSOneway(bs1);
            batchC2.opByteSOneway(bs1);
            batchC2.opByteSOneway(bs1);
            batchC1.ice_getConnection().flushBatchRequests(CompressBatch.No);

            batchC1.opByteSOneway(bs1);
            batchC1.opByteSOneway(bs1);
            batchC1.opByteSOneway(bs1);
            batchC1.ice_getConnection()
                .flushBatchRequests(CompressBatch.BasedOnProxy);

            batchC1.opByteSOneway(bs1);
            batchC2.opByteSOneway(bs1);
            batchC1.opByteSOneway(bs1);
            batchC1.ice_getConnection()
                .flushBatchRequests(CompressBatch.BasedOnProxy);

            batchC1.opByteSOneway(bs1);
            batchC3.opByteSOneway(bs1);
            batchC1.opByteSOneway(bs1);
            batchC1.ice_getConnection()
                .flushBatchRequests(CompressBatch.BasedOnProxy);
        }
    }

    private BatchOneways() {}
}
