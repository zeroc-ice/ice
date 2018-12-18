// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

import java.io.PrintWriter;

import com.zeroc.Ice.Util;
import com.zeroc.Ice.LocalException;

import test.Ice.operations.Test.MyClassPrx;

class BatchOnewaysAMI
{
    private static class Callback
    {
        Callback()
        {
            _called = false;
        }

        public synchronized void check()
        {
            while(!_called)
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                }
            }

            _called = false;
        }

        public synchronized void called()
        {
            assert (!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    static void batchOneways(MyClassPrx p, PrintWriter out)
    {
        final com.zeroc.Ice.Communicator communicator = p.ice_getCommunicator();
        final com.zeroc.Ice.Properties properties = communicator.getProperties();
        final byte[] bs1 = new byte[10 * 1024];

        MyClassPrx batch = p.ice_batchOneway();
        batch.ice_flushBatchRequestsAsync().join(); // Empty flush

        {
            test(batch.ice_flushBatchRequestsAsync().isDone()); // Empty flush
            test(Util.getInvocationFuture(batch.ice_flushBatchRequestsAsync()).isSent()); // Empty flush
            test(Util.getInvocationFuture(batch.ice_flushBatchRequestsAsync()).sentSynchronously()); // Empty flush
        }

        for(int i = 0; i < 30; ++i)
        {
            batch.opByteSOnewayAsync(bs1).whenComplete((result, ex) ->
                {
                    test(ex == null);
                });
        }

        int count = 0;
        while(count < 27) // 3 * 9 requests auto-flushed.
        {
            count += p.opByteSOnewayCallCount();
            try
            {
                Thread.sleep(10);
            }
            catch(InterruptedException ex)
            {
            }
        }

        final boolean bluetooth = properties.getProperty("Ice.Default.Protocol").indexOf("bt") == 0;
        if(batch.ice_getConnection() != null && !bluetooth)
        {
            MyClassPrx batch2 = p.ice_batchOneway();

            batch.ice_pingAsync();
            batch2.ice_pingAsync();
            batch.ice_flushBatchRequestsAsync().join();
            batch.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);
            batch.ice_pingAsync();
            batch2.ice_pingAsync();

            batch.ice_getConnection();
            batch2.ice_getConnection();

            batch.ice_pingAsync();
            batch.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);
            test(!batch.ice_pingAsync().isCompletedExceptionally());
            test(!batch2.ice_pingAsync().isCompletedExceptionally());
        }

        com.zeroc.Ice.Identity identity = new com.zeroc.Ice.Identity();
        identity.name = "invalid";
        com.zeroc.Ice.ObjectPrx batch3 = batch.ice_identity(identity);
        batch3.ice_pingAsync();
        batch3.ice_flushBatchRequestsAsync().join();

        // Make sure that a bogus batch request doesn't cause troubles to other ones.
        batch3.ice_pingAsync();
        batch.ice_pingAsync();
        batch.ice_flushBatchRequestsAsync().join();
        batch.ice_pingAsync();
    }
}
