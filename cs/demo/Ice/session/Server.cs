// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Threading;
using Demo;

public class Server : Ice.Application
{
    public override int run(string[] args)
    {
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("SessionFactory");

        ReapThread reaper = new ReapThread();
        Thread reaperThread = new Thread(new ThreadStart(reaper.run));
        reaperThread.Start();

        adapter.add(new SessionFactoryI(reaper), communicator().stringToIdentity("SessionFactory"));
        adapter.activate();
        communicator().waitForShutdown();

        reaper.terminate();
        reaperThread.Join();

        return 0;
    }

    public static void Main(string[] args)
    {
        Server app = new Server();
        int status = app.main(args, "config.server");
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}
