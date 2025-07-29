// Copyright (c) ZeroC, Inc.

package test.Ice.operations;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

import test.Ice.operations.Test.MyClassPrx;

import java.io.PrintWriter;

class BatchOnewaysAMI {
    private static class Callback {
        Callback() {
            _called = false;
        }

        public synchronized void check() {
            while (!_called) {
                try {
                    wait();
                } catch (InterruptedException ex) {}
            }

            _called = false;
        }

        public synchronized void called() {
            assert (!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    static void batchOneways(MyClassPrx p, PrintWriter out) {
        final Communicator communicator = p.ice_getCommunicator();
        final Properties properties = communicator.getProperties();
        final byte[] bs1 = new byte[10 * 1024];

        MyClassPrx batch = p.ice_batchOneway();
        batch.ice_flushBatchRequestsAsync().join(); // Empty flush

        {
            test(batch.ice_flushBatchRequestsAsync().isDone()); // Empty flush
            test(
                Util.getInvocationFuture(batch.ice_flushBatchRequestsAsync())
                    .isSent()); // Empty flush
            test(
                Util.getInvocationFuture(batch.ice_flushBatchRequestsAsync())
                    .sentSynchronously()); // Empty flush
        }

        for (int i = 0; i < 30; i++) {
            batch.opByteSOnewayAsync(bs1)
                .whenComplete(
                    (result, ex) -> {
                        test(ex == null);
                    });
        }

        int count = 0;
        while (count < 27 /* 3*9 requests auto-flushed */) {
            count += p.opByteSOnewayCallCount();
            try {
                Thread.sleep(10);
            } catch (InterruptedException ex) {}
        }

        final boolean bluetooth =
            properties.getIceProperty("Ice.Default.Protocol").indexOf("bt") == 0;
        if (batch.ice_getConnection() != null && !bluetooth) {
            MyClassPrx batch2 = p.ice_batchOneway();

            batch.ice_pingAsync();
            batch2.ice_pingAsync();
            batch.ice_flushBatchRequestsAsync().join();
            batch.ice_getConnection().close();
            batch.ice_pingAsync();
            batch2.ice_pingAsync();

            batch.ice_getConnection();
            batch2.ice_getConnection();

            batch.ice_pingAsync();
            batch.ice_getConnection().close();
            test(!batch.ice_pingAsync().isCompletedExceptionally());
            test(!batch2.ice_pingAsync().isCompletedExceptionally());
        }

        Identity identity = new Identity();
        identity.name = "invalid";
        ObjectPrx batch3 = batch.ice_identity(identity);
        batch3.ice_pingAsync();
        batch3.ice_flushBatchRequestsAsync().join();

        // Make sure that a bogus batch request doesn't cause troubles to other ones.
        batch3.ice_pingAsync();
        batch.ice_pingAsync();
        batch.ice_flushBatchRequestsAsync().join();
        batch.ice_pingAsync();
    }

    private BatchOnewaysAMI() {}
}
