// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.udp;

import test.Ice.udp.Test.*;

public final class TestIntfI extends _TestIntfDisp
{
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

    public void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
