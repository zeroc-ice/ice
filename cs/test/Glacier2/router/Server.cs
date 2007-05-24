// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;

public class Server : Ice.Application
{
    public override int run(string[] args)
    {
        communicator().getProperties().setProperty("CallbackAdapter.Endpoints", "tcp -p 12010 -t 10000");
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("CallbackAdapter");
        adapter.add(new CallbackI(),
                    communicator().stringToIdentity("c1/callback")); // The test allows "c1" as category.
        adapter.add(new CallbackI(),
                    communicator().stringToIdentity("c2/callback")); // The test allows "c2" as category.
        adapter.add(new CallbackI(),
                    communicator().stringToIdentity("c3/callback")); // The test rejects "c3" as category.
        adapter.add(new CallbackI(),
                    communicator().stringToIdentity("_userid/callback")); // The test allows the prefixed userid.
        adapter.activate();
        communicator().waitForShutdown();
        return 0;
    }

    public static void Main(string[] args)
    {
        Debug.Listeners.Add(new ConsoleTraceListener());

        Server app = new Server();
        int status = app.main(args);
        if(status != 0)
        {
            Environment.Exit(status);
        }
    }
}
