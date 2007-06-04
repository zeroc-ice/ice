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
            communicator().shutdown();
        }
    }

    public int
    run(String[] args)
    {
        setInterruptHook(new ShutdownHook());

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Hello");
        _workQueue = new WorkQueue();

        final HelloI helloServant = new HelloI(_workQueue);

        Ice.ServantLocator servantLocator = new Ice.ServantLocator()
            {
                public Ice.Object locate(Ice.Current curr, Ice.LocalObjectHolder cookie)
                {
                    return helloServant;
                }

                public void finished(Ice.Current curr, Ice.Object servant, java.lang.Object cookie)
                {
                    // nothing
                }

                public void deactivate(String category)
                {
                    System.err.println("deactivating servant locator");
                }
            };


        adapter.addServantLocator(servantLocator, "");

        _workQueue.start();
        adapter.activate();

        communicator().waitForShutdown();

        try
        {
            _workQueue.join();
        }
        catch(java.lang.InterruptedException ex)
        {
        }

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
