// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Server extends Ice.Application
{
    @Override
    public int
    run(String[] args)
    {
        if(args.length > 0)
        {
            System.err.println(appName() + ": too many arguments");
            return 1;
        }
        
        //
        // If ^C is pressed we want to interrupt all running upcalls from the
        // dispatcher and destroy the communicator.
        //
        setInterruptHook(new Thread() {
            @Override
            public void
            run()
            {
                //
                // Call shutdownNow on the executor. This interrupts all
                // executor threads causing any running servant dispatch threads
                // to terminate quickly.
                //
                _executor.shutdownNow();
                try
                {
                    communicator().shutdown();
                }
                catch(Ice.LocalException ex)
                {
                    ex.printStackTrace();
                }
            }
        });

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TaskManager");
        adapter.add(new TaskManagerI(_executor), communicator().stringToIdentity("manager"));
        adapter.activate();
        communicator().waitForShutdown();
        
        return 0;
    }

    public static void
    main(String[] args)
    {
        final Server app = new Server();

        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties();
        initData.properties.load("config.server");

        //
        // This demo uses a dispatcher to execute any invocations on the server.
        // By using an executor it is straightforward to interrupt any servant
        // dispatch threads by using ExecutorService.shutdownNow.
        //
        initData.dispatcher = new Ice.Dispatcher() {
            @Override
            public void dispatch(Runnable runnable, Ice.Connection con)
            {
                app.getExecutor().submit(runnable);
            }
        };
        
        int status = app.main("Server", args, initData);
        System.exit(status);
    }
    
    ExecutorService getExecutor()
    {
        return _executor;
    }

    private ExecutorService _executor = Executors.newFixedThreadPool(5);
}
