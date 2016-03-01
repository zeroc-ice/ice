// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.adapterDeactivation;

import test.Ice.adapterDeactivation.Test.*;

public final class TestI extends _TestIntfDisp
{
    public void
    _transient(Ice.Current current)
    {
        Ice.Communicator communicator = current.adapter.getCommunicator();

        Ice.ObjectAdapter adapter =
            communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default -p 9999");
        adapter.activate();
        adapter.destroy();
    }

    public void
    deactivate(Ice.Current current)
    {
        current.adapter.deactivate();
        try
        {
            Thread.sleep(100);
        }
        catch(InterruptedException ex)
        {
        }
    }
}
