// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;

public class ContextI : ContextDisp_
{
    public override void call(Ice.Current current)
    {
        System.Console.Out.Write("Type = ");
        string type = current.ctx["type"];
        if(type != null)
        {
            System.Console.Out.Write(type);
        }
        else
        {
            System.Console.Out.Write("None");
        }
        System.Console.Out.WriteLine("");
    }

    public override void shutdown(Ice.Current current)
    {
        System.Console.Out.WriteLine("Shutting down...");
        current.adapter.getCommunicator().shutdown();
    }
}
