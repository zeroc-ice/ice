// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;

public sealed class TestIntfI : Test.TestIntfDisp_
{
    public override void ping(Test.PingReplyPrx reply, Ice.Current current)
    {
        System.Console.WriteLine("ping");
        try
        {
            reply.reply();
        }
        catch(Ice.LocalException)
        {
            Debug.Assert(false);
        }
    }

    public override void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
