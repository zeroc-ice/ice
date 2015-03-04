// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
        byte[] tbs1 = new byte[10  * 1024];
        byte[] tbs2 = new byte[99  * 1024];
        byte[] tbs3 = new byte[100 * 1024];
        Test.ByteS bs1 = new Test.ByteS(tbs1);
        Test.ByteS bs2 = new Test.ByteS(tbs2);
        Test.ByteS bs3 = new Test.ByteS(tbs3);

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

        try
        {
            p.opByteSOneway(bs3);
            test(false);
        }
        catch(Ice.MemoryLimitException)
        {
            test(true);
        }

        Test.MyClassPrx batch = Test.MyClassPrxHelper.uncheckedCast(p.ice_batchOneway());

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
