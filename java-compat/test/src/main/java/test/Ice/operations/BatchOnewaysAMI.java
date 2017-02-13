// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

import java.io.PrintWriter;

import Ice.LocalException;
import test.Ice.operations.Test.Callback_MyClass_opByteSOneway;
import test.Ice.operations.Test.MyClassPrx;
import test.Ice.operations.Test.MyClassPrxHelper;

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
        final byte[] bs1 = new byte[10 * 1024];

        MyClassPrx batch = MyClassPrxHelper.uncheckedCast(p.ice_batchOneway());
        batch.end_ice_flushBatchRequests(batch.begin_ice_flushBatchRequests()); // Empty flush

        test(batch.begin_ice_flushBatchRequests().isCompleted()); // Empty flush
        test(batch.begin_ice_flushBatchRequests().isSent()); // Empty flush
        test(batch.begin_ice_flushBatchRequests().sentSynchronously()); // Empty flush

        for(int i = 0; i < 30; ++i)
        {
            batch.begin_opByteSOneway(bs1, new Callback_MyClass_opByteSOneway()
            {
                @Override
                public void exception(LocalException ex)
                {
                    test(false);
                }

                @Override
                public void response()
                {
                }
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

        if(batch.ice_getConnection() != null)
        {
            MyClassPrx batch2 = MyClassPrxHelper.uncheckedCast(p.ice_batchOneway());

            batch.begin_ice_ping();
            batch2.begin_ice_ping();
            batch.end_ice_flushBatchRequests(batch.begin_ice_flushBatchRequests());
            batch.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
            batch.begin_ice_ping();
            batch2.begin_ice_ping();

            batch.ice_getConnection();
            batch2.ice_getConnection();

            batch.begin_ice_ping();
            batch.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
            batch.begin_ice_ping().throwLocalException();
            batch2.begin_ice_ping().throwLocalException();
        }

        Ice.Identity identity = new Ice.Identity();
        identity.name = "invalid";
        Ice.ObjectPrx batch3 = batch.ice_identity(identity);
        batch3.begin_ice_ping();
        batch3.end_ice_flushBatchRequests(batch3.begin_ice_flushBatchRequests());

        // Make sure that a bogus batch request doesn't cause troubles to other
        // ones.
        batch3.begin_ice_ping();
        batch.begin_ice_ping();
        batch.end_ice_flushBatchRequests(batch.begin_ice_flushBatchRequests());
        batch.begin_ice_ping();
    }
}
