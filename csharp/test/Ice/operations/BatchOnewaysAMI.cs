// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
                batch.end_ice_flushBatchRequests(batch.begin_ice_flushBatchRequests());

                test(batch.begin_ice_flushBatchRequests().isSent());
                test(batch.begin_ice_flushBatchRequests().isCompleted_());
                test(batch.begin_ice_flushBatchRequests().sentSynchronously());

                for (int i = 0; i < 30; ++i)
                {
                    batch.begin_opByteSOneway(bs1).whenCompleted(
                        () =>
                        {
                        },
                        (Ice.Exception ex) =>
                        {
                            test(false);
                        });
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

                    batch1.begin_ice_ping();
                    batch2.begin_ice_ping();
                    batch1.end_ice_flushBatchRequests(batch1.begin_ice_flushBatchRequests());
                    batch1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                    batch1.begin_ice_ping();
                    batch2.begin_ice_ping();

                    batch1.ice_getConnection();
                    batch2.ice_getConnection();

                    batch1.begin_ice_ping();
                    batch1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);

                    batch1.begin_ice_ping();
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
    }
}
