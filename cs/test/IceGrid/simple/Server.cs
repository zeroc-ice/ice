// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;

public class Server : Ice.Application
{
    public override int run(string[] args)
    {
        args = communicator().getProperties().parseCommandLineOptions("TestAdapter", args);
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        adapter.add(new TestI(adapter), communicator().stringToIdentity("test"));
        shutdownOnInterrupt();
        try
        {
            adapter.activate();
        }
        catch(Ice.ObjectAdapterDeactivatedException)
        {
        }
        communicator().waitForShutdown();
        return 0;
    }

    public static void Main(string[] args)
    {
        Debug.Listeners.Add(new ConsoleTraceListener());

        Server server = new Server();
        int status = server.main(args);
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}
