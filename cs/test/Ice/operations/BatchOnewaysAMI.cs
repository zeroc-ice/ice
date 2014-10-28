// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;

class BatchOnewaysAMI
{
    private static void test(bool b)
    {
        if(!b)
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
            lock(this)
            {
                while(!_called)
                {
                    System.Threading.Monitor.Wait(this);
                }

                _called = false;
            }
        }

        public void called()
        {
            lock(this)
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
        byte[] bs1 = new byte[10  * 1024];
        byte[] bs2 = new byte[99  * 1024];
        byte[] bs3 = new byte[100 * 1024];

        Callback cb = new Callback();
        p.begin_opByteSOneway(bs1).whenCompleted(
            () =>
            {
                cb.called();
            },
            (Ice.Exception ex) =>
            {
                test(false);
            });
        cb.check();

        p.begin_opByteSOneway(bs2).whenCompleted(
            () =>
            {
                cb.called();
            },
            (Ice.Exception ex) =>
            {
                test(false);
            });
        cb.check();

        p.begin_opByteSOneway(bs3).whenCompleted(
            () =>
            {
                test(false);
            },
            (Ice.Exception ex) =>
            {
                test(ex is Ice.MemoryLimitException);
                cb.called();
            });
        cb.check();

        Test.MyClassPrx batch = Test.MyClassPrxHelper.uncheckedCast(p.ice_batchOneway());
        batch.end_ice_flushBatchRequests(batch.begin_ice_flushBatchRequests());

        for(int i = 0 ; i < 30 ; ++i)
        {
            p.begin_opByteSOneway(bs1).whenCompleted(
                () =>
                {
                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
        }

        if(batch.ice_getConnection() != null)
        {
            batch.ice_getConnection().end_flushBatchRequests(batch.ice_getConnection().begin_flushBatchRequests());

            Test.MyClassPrx batch2 = Test.MyClassPrxHelper.uncheckedCast(p.ice_batchOneway());

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
            batch.begin_ice_ping().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception ex) =>
                {
                    test(ex is Ice.CloseConnectionException);
                });
            batch2.begin_ice_ping().whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception ex) =>
                {
                    test(ex is Ice.CloseConnectionException);
                });

            batch.begin_ice_ping();
            batch2.begin_ice_ping();
        }

        Ice.Identity identity = new Ice.Identity();
        identity.name = "invalid";
        Ice.ObjectPrx batch3 = batch.ice_identity(identity);
        batch3.begin_ice_ping();
        batch3.end_ice_flushBatchRequests(batch3.begin_ice_flushBatchRequests());

        // Make sure that a bogus batch request doesn't cause troubles to other ones.
        batch3.begin_ice_ping();
        batch.begin_ice_ping();
        batch.end_ice_flushBatchRequests(batch.begin_ice_flushBatchRequests());
        batch.begin_ice_ping();
    }
}
