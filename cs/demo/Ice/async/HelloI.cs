// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Demo;

public class HelloI : HelloDisp_
{
    public HelloI(WorkQueue workQueue)
    {
        _workQueue = workQueue;
    }

    public override void sayHello_async(AMD_Hello_sayHello cb, int delay, Ice.Current current)
    {
       if(delay == 0)
       {
           Console.Out.WriteLine("Hello World!");
           cb.ice_response();
       }
       else
       {
           _workQueue.Add(cb, delay);
       }
    }

    public override void shutdown(Ice.Current current)
    {
        _workQueue.destroy();
        current.adapter.getCommunicator().shutdown();
    }

    private WorkQueue _workQueue;
}
