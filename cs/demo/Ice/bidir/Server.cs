// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;
using System.Threading;

public class Server : Ice.Application
{
    public override int run(string[] args)
    {
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Callback.Server");
        CallbackSenderI sender = new CallbackSenderI(communicator());
        adapter.add(sender, communicator().stringToIdentity("sender"));
        adapter.activate();

        Thread t = new Thread(new ThreadStart(sender.Run));
        t.Start();

        try
        {
            communicator().waitForShutdown();
        }
        finally
        {
            sender.destroy();
            t.Join();
        }

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
