// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceAsyncServer")]
[assembly: AssemblyDescription("Ice async demo server")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server
{
    public class App : Ice.Application
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

        private WorkQueue _workQueue;
    }

    public static int Main(string[] args)
    {
        App app = new App();
        return app.main(args, "config.server");
    }
}
