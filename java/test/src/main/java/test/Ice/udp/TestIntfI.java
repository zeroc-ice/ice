// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.udp;

import test.Ice.udp.Test.*;

public final class TestIntfI implements TestIntf
{
    @Override
    public void ping(PingReplyPrx reply, com.zeroc.Ice.Current current)
    {
        try
        {
            reply.reply();
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            assert(false);
        }
    }

    @Override
    public void sendByteSeq(byte[] seq, PingReplyPrx reply, com.zeroc.Ice.Current current)
    {
        try
        {
            reply.reply();
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            assert(false);
        }
    }

    @Override
    public void pingBiDir(com.zeroc.Ice.Identity id, com.zeroc.Ice.Current current)
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
                TestIntfPrx.uncheckedCast(current.con.createProxy(id)).sendByteSeq(seq, null);
            }
            catch(com.zeroc.Ice.DatagramLimitException ex)
            {
                // Expected.
            }

            PingReplyPrx.uncheckedCast(current.con.createProxy(id)).reply();
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            assert(false);
        }
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
