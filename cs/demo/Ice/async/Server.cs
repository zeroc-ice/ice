// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Server : Ice.Application
{
    public override int run(string[] args)
    {
        if(args.Length > 0)
        {
            System.Console.Error.WriteLine(appName() + ": too many arguments");
            return 1;
        }

        callbackOnInterrupt();

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Hello");
        _workQueue = new WorkQueue();
        adapter.add(new HelloI(_workQueue), communicator().stringToIdentity("hello"));

        _workQueue.Start();
        adapter.activate();

        communicator().waitForShutdown();
        _workQueue.Join();
        return 0;
    }

    public override void interruptCallback(int sig)
    {
        _workQueue.destroy();
        communicator().shutdown();
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

    private WorkQueue _workQueue;
}
