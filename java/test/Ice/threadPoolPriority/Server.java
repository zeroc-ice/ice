// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.threadPoolPriority;

public class Server extends test.Util.Application
{
    public int run(String[] args)
    {
        //
        // We don't want connection warnings because of the timeout test.
        //
        communicator().getProperties().setProperty("Ice.Warn.Connections", "0");
        communicator().getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010 -t 10000:udp");
        communicator().getProperties().setProperty("Ice.ThreadPool.Server.ThreadPriority", "10");

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        adapter.add(new PriorityI(), communicator().stringToIdentity("test"));
        adapter.activate();

        communicator().waitForShutdown();
        return 0;
    }

    public static void main(String[] args)
    {
        Server c = new Server();
        int status = c.main("Server", args);

        System.gc();
        System.exit(status);
    }
}
