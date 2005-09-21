// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Demo;

public class HelloI : HelloDisp_
{
    public override void sayHello(Ice.Current current)
    {
        Console.WriteLine("Hello World!");
    }

    public override void shutdown(Ice.Current current)
    {
        Console.WriteLine("Shutting down...");
        current.adapter.getCommunicator().shutdown();
    }
}
