// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Server extends Ice.Application
{
    public int
    run(String[] args)
    {
        if(args.length > 0)
        {
            System.err.println(appName() + ": too many arguments");
            return 1;
        }

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("SessionFactory");

        java.util.concurrent.ScheduledExecutorService executor = java.util.concurrent.Executors.newScheduledThreadPool(1);
        ReapTask reaper = new ReapTask();
        executor.scheduleAtFixedRate(reaper, 1, 1, java.util.concurrent.TimeUnit.SECONDS);

        adapter.add(new SessionFactoryI(reaper), communicator().stringToIdentity("SessionFactory"));
        adapter.activate();
        communicator().waitForShutdown();

        executor.shutdown();
        reaper.terminate();

        return 0;
    }

    public static void
    main(String[] args)
    {
        Server app = new Server();
        int status = app.main("Server", args, "config.server");
        System.exit(status);
    }
}
