// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class HelloI extends _HelloDisp
{
    public 
    HelloI(WorkQueue workQueue)
    {
        _workQueue = workQueue;
    }

    public void
    sayHello_async(AMD_Hello_sayHello cb, int delay, Ice.Current current)
    {
        if(delay == 0)
        {
            System.out.println("Hello World!");
            cb.ice_response();
        }
        else
        {
            _workQueue.add(cb, delay);
        }
    }

    public void
    shutdown(Ice.Current current)
    {
        System.out.println("Shutting down...");

        _workQueue.destroy();
        current.adapter.getCommunicator().shutdown();
    }

    private WorkQueue _workQueue;
}
