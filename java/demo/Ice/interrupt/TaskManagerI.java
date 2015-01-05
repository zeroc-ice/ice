// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import java.util.concurrent.ExecutorService;

import Demo.*;

public class TaskManagerI extends _TaskManagerDisp
{
    public TaskManagerI(ExecutorService executor)
    {
        _executor = executor;
    }

    @Override
    public void
    run(int id, Ice.Current current)
    {
        System.out.println("starting task " + id);
        // Sleep for 10 seconds.
        try
        {
            Thread.sleep(10000);
            System.out.println("stopping task " + id);
        }
        catch(InterruptedException ex)
        {
            //
            // We are done, the server is shutting down.
            //
            System.out.println("interrupted task " + id);
        }
    }

    @Override
    public void
    shutdown(Ice.Current current)
    {
        System.out.println("Shutting down...");
         //
        // Call shutdownNow on the executor. This interrupts all
        // executor threads causing any running upcalls to terminate
        // quickly.
        //
        _executor.shutdownNow();
        current.adapter.getCommunicator().shutdown();
    }
    
    private ExecutorService _executor;
}
