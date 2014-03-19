// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import FilesystemI.*;

class Server extends Ice.Application
{
    public int
    run(String[] args)
    {
        //
        // Terminate cleanly on receipt of a signal.
        //
        shutdownOnInterrupt();
        Ice.Properties properties = communicator().getProperties();

        //
        // Create an object adapter
        //
        Ice.ObjectAdapter adapter = communicator().createObjectAdapterWithEndpoints(
            "LifecycleFilesystem", "default -h localhost -p 10000");

        //
        // Create the root directory.
        //
        DirectoryI root = new DirectoryI();
        Ice.Identity id = new Ice.Identity();
        id.name = "RootDir";
        adapter.add(root, id);

        //
        // All objects are created, allow client requests now.
        //
        adapter.activate();

        //
        // Wait until we are done.
        //
        communicator().waitForShutdown();
        if(interrupted())
        {
            System.err.println(appName() + ": received signal, shutting down");
        }

        return 0;
    }

    static public void
    main(String[] args)
    {
        Server app = new Server();
        app.main("demo.book.lifecycle.Server", args);
    }
}
