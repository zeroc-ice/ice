// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

public sealed class TestI : _TestIntfDisp
{
    public override void transient(Ice.Current current)
    {
        Ice.Communicator communicator = current.adapter.getCommunicator();

        Ice.ObjectAdapter adapter =
            communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default -p 9999");
        adapter.activate();
        adapter.deactivate();
        adapter.waitForDeactivate();
    }

    public override void deactivate(Ice.Current current)
    {
        current.adapter.deactivate();
	System.Threading.Thread.Sleep(new System.TimeSpan(10 * 1000));
    }
}
