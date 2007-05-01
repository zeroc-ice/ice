// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
    batchOneways(Test.MyClassPrx p)
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

        Test.MyClassPrx batch = Test.MyClassPrxHelper.uncheckedCast(p.ice_batchOneway());

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
    }
}
