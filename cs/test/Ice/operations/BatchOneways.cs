// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
            throw new System.Exception();
        }
    }

    internal static void batchOneways(Test.MyClassPrx p)
    {
        byte[] bs1 = new byte[10  * 1024];
        byte[] bs2 = new byte[99  * 1024];

        try
        {
            p.opByteSOneway(bs1);
            test(true);
        }
        catch(Ice.MemoryLimitException)
        {
            test(false);
        }

        try
        {
            p.opByteSOneway(bs2);
            test(true);
        }
        catch(Ice.MemoryLimitException)
        {
            test(false);
        }

        Test.MyClassPrx batch = Test.MyClassPrxHelper.uncheckedCast(p.ice_batchOneway());
        batch.ice_flushBatchRequests();

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
        while(count != 27) // 3 * 9 requests auto-flushed.
        {
            count += p.opByteSOnewayCallCount();
            System.Threading.Thread.Sleep(10);
        }

        if(batch.ice_getConnection() != null)
        {
            batch.ice_getConnection().flushBatchRequests();

            Test.MyClassPrx batch2 = Test.MyClassPrxHelper.uncheckedCast(p.ice_batchOneway());

            batch.ice_ping();
            batch2.ice_ping();
            batch.ice_flushBatchRequests();
            batch.ice_getConnection().close(false);
            batch.ice_ping();
            batch2.ice_ping();

            batch.ice_getConnection();
            batch2.ice_getConnection();

            batch.ice_ping();
            batch.ice_getConnection().close(false);
            try
            {
                batch.ice_ping();
                test(false);
            }
            catch(Ice.CloseConnectionException)
            {
            }

            try
            {
                batch2.ice_ping();
                test(false);
            }
            catch(Ice.CloseConnectionException)
            {
            }
            batch.ice_ping();
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
    }
}
