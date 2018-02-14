// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Threading;

internal class ActivateAdapterThread
{
    internal ActivateAdapterThread(Ice.ObjectAdapter adapter, int timeout)
    {
        _adapter = adapter;
        _timeout = timeout;
    }

    internal void run()
    {
        _adapter.waitForHold();
        Thread.Sleep(_timeout);
        _adapter.activate();
    }

    private Ice.ObjectAdapter _adapter;
    int _timeout;
}

internal class TimeoutI : Test.TimeoutDisp_
{
    public override void op(Ice.Current current)
    {
    }

    public override void sendData(byte[] seq, Ice.Current current)
    {
    }

    public override void sleep(int to, Ice.Current current)
    {
        Thread.Sleep(to);
    }

    public override void holdAdapter(int to, Ice.Current current)
    {
        current.adapter.hold();
        ActivateAdapterThread act = new ActivateAdapterThread(current.adapter, to);
        Thread thread = new Thread(new ThreadStart(act.run));
        thread.Start();
    }

    public override void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
