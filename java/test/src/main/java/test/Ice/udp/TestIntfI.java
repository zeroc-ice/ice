// Copyright (c) ZeroC, Inc.

package test.Ice.udp;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.DatagramLimitException;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.LocalException;

import test.Ice.udp.Test.*;

public final class TestIntfI implements TestIntf {
    @Override
    public void ping(PingReplyPrx reply, Current current) {
        try {
            reply.reply();
        } catch (LocalException ex) {
            assert false;
        }
    }

    @Override
    public void sendByteSeq(byte[] seq, PingReplyPrx reply, Current current) {
        try {
            reply.reply();
        } catch (LocalException ex) {
            assert false;
        }
    }

    @Override
    public void pingBiDir(Identity id, Current current) {
        try {
            //
            // Ensure sending too much data doesn't cause the UDP connection to be closed.
            //
            try {
                byte[] seq = new byte[32 * 1024];
                TestIntfPrx.uncheckedCast(current.con.createProxy(id)).sendByteSeq(seq, null);
            } catch (DatagramLimitException ex) {
                // Expected.
            }

            PingReplyPrx.uncheckedCast(current.con.createProxy(id)).reply();
        } catch (LocalException ex) {
            assert false;
        }
    }

    @Override
    public void shutdown(Current current) {
        current.adapter.getCommunicator().shutdown();
    }
}
