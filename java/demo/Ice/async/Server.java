// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Server extends Ice.Application
{
    class ShutdownHook extends Thread
    {
        public void
        run()
        {
            _workQueue.destroy();
            try
            {
                _workQueue.join();
            }
            catch(java.lang.InterruptedException ex)
            {
            }

            try
            {
                communicator().destroy();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
            }
        }
    }

    public int
    run(String[] args)
    {
        setInterruptHook(new ShutdownHook());

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Hello");
        _workQueue = new WorkQueue();
        adapter.add(new HelloI(_workQueue), communicator().stringToIdentity("hello"));

        _workQueue.start();
        adapter.activate();

        communicator().waitForShutdown();
        return 0;
    }

    public static void
    main(String[] args)
    {
        Server app = new Server();
        int status = app.main("Server", args, "config.server");
        System.exit(status);
    }

    private WorkQueue _workQueue;
}
