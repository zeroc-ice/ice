// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.udp;

import test.Ice.udp.Test.*;

public final class TestIntfI extends _TestIntfDisp
{
    @Override
    public void ping(PingReplyPrx reply, Ice.Current current)
    {
        try
        {
            reply.reply();
        }
        catch(Ice.LocalException ex)
        {
            assert(false);
        }
    }

    @Override
    public void sendByteSeq(byte[] seq, PingReplyPrx reply, Ice.Current current)
    {
        try
        {
            reply.reply();
        }
        catch(Ice.LocalException ex)
        {
            assert(false);
        }
    }

    @Override
    public void pingBiDir(Ice.Identity id, Ice.Current current)
    {
        try
        {
            //
            // Ensure sending too much data doesn't cause the UDP connection
            // to be closed.
            //
            try
            {
                byte[] seq = new byte[32 * 1024];
                TestIntfPrxHelper.uncheckedCast(current.con.createProxy(id)).sendByteSeq(seq, null);
            }
            catch(Ice.DatagramLimitException ex)
            {
                // Expected.
            }

            PingReplyPrxHelper.uncheckedCast(current.con.createProxy(id)).reply();
        }
        catch(Ice.LocalException ex)
        {
            assert(false);
        }
    }

    @Override
    public void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
