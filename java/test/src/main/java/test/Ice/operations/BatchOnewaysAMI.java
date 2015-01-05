// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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
        final byte[] bs2 = new byte[99 * 1024];

        final Callback cb = new Callback();
        p.begin_opByteSOneway(bs1, new Callback_MyClass_opByteSOneway()
        {
            @Override
            public void exception(LocalException ex)
            {
                test(false);
            }

            @Override
            public void response()
            {
                cb.called();
            }
        });
        cb.check();
        p.begin_opByteSOneway(bs2, new Callback_MyClass_opByteSOneway()
        {
            @Override
            public void exception(LocalException ex)
            {
                test(false);
            }

            @Override
            public void response()
            {
                cb.called();
            }
        });
        cb.check();

        MyClassPrx batch = MyClassPrxHelper.uncheckedCast(p.ice_batchOneway());
        batch.end_ice_flushBatchRequests(batch.begin_ice_flushBatchRequests());

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

        if(batch.ice_getConnection() != null)
        {
            batch.ice_getConnection().end_flushBatchRequests(batch.ice_getConnection().begin_flushBatchRequests());

            MyClassPrx batch2 = MyClassPrxHelper.uncheckedCast(p.ice_batchOneway());

            batch.begin_ice_ping();
            batch2.begin_ice_ping();
            batch.end_ice_flushBatchRequests(batch.begin_ice_flushBatchRequests());
            batch.ice_getConnection().close(false);
            batch.begin_ice_ping();
            batch2.begin_ice_ping();

            batch.ice_getConnection();
            batch2.ice_getConnection();

            batch.begin_ice_ping();
            batch.ice_getConnection().close(false);
            batch.begin_ice_ping(new Ice.Callback_Object_ice_ping()
            {

                @Override
                public void response()
                {
                    test(false);
                }

                @Override
                public void exception(LocalException ex)
                {
                    test(ex instanceof Ice.CloseConnectionException);
                    cb.called();
                }

            });
            cb.check();
            batch2.begin_ice_ping(new Ice.Callback_Object_ice_ping()
            {

                @Override
                public void response()
                {
                    test(false);
                }

                @Override
                public void exception(LocalException ex)
                {
                    test(ex instanceof Ice.CloseConnectionException);
                    cb.called();
                }

            });
            cb.check();
            batch.begin_ice_ping();
            batch2.begin_ice_ping();
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
