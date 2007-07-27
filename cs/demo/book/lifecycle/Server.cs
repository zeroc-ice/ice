// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using FilesystemI;

class FilesystemApp : Ice.Application
{
    public override int run(string[] args)
    {
        // Terminate cleanly on receipt of a signal.
        //
        shutdownOnInterrupt();

        // Create an object adapter
        //
        Ice.ObjectAdapter adapter = communicator().createObjectAdapterWithEndpoints(
                                                        "LifecycleFilesystem", "default -p 10000");

        // Create the root directory.
        //
        new DirectoryI(adapter);

        // All objects are created, allow client requests now.
        //
        adapter.activate();

        //
        // Wait until we are done.
        communicator().waitForShutdown();
        if(interrupted())
        {
            System.Console.Error.WriteLine(appName() + ": received signal, shutting down");
        }

        return 0;
    }
}

public class Server
{
    static public void Main(string[] args)
    {
        FilesystemApp app = new FilesystemApp();
        app.main(args);
    }
}
