// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

import java.io.PrintWriter;

import test.Ice.operations.Test.MyClassPrx;
import test.Ice.operations.Test.MyClassPrxHelper;

class BatchOneways
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    static void
    batchOneways(MyClassPrx p, PrintWriter out)
    {
        final byte[] bs1 = new byte[10  * 1024];
        final byte[] bs2 = new byte[99  * 1024];
        final byte[] bs3 = new byte[100 * 1024];

        try
        {
            p.opByteSOneway(bs1);
        }
        catch(Ice.MemoryLimitException ex)
        {
            test(false);
        }

        try
        {
            p.opByteSOneway(bs2);
        }
        catch(Ice.MemoryLimitException ex)
        {
            test(false);
        }

        try
        {
            p.opByteSOneway(bs3);
            test(false);
        }
        catch(Ice.MemoryLimitException ex)
        {
        }

        MyClassPrx batch = MyClassPrxHelper.uncheckedCast(p.ice_batchOneway());

        for(int i = 0 ; i < 30 ; ++i)
        {
            try
            {
                batch.opByteSOneway(bs1);
            }
            catch(Ice.MemoryLimitException ex)
            {
                test(false);
            }
        }

        batch.ice_getConnection().flushBatchRequests();

        MyClassPrx batch2 = MyClassPrxHelper.uncheckedCast(p.ice_batchOneway());

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
        catch(Ice.CloseConnectionException ex)
        {
        }
        try
        {
            batch2.ice_ping();
            test(false);
        }
        catch(Ice.CloseConnectionException ex)
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
