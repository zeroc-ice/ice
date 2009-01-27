// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public final class TestIntfI extends Test._TestIntfDisp
{
    public void ping(Test.PingReplyPrx reply, Ice.Current current)
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

    public void sendByteSeq(byte[] seq, Test.PingReplyPrx reply, Ice.Current current)
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
