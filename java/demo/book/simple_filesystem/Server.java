// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Filesystem.*;

public class Server extends Ice.Application
{
    public int
    run(String[] args)
    {
        //
        // Terminate cleanly on receipt of a signal
        //
        shutdownOnInterrupt();

        //
        // Create an object adapter.
        //
        Ice.ObjectAdapter adapter =
            communicator().createObjectAdapterWithEndpoints("SimpleFilesystem", "default -h localhost -p 10000");

        //
        // Create the root directory (with name "/" and no parent)
        //
        DirectoryI root = new DirectoryI(communicator(), "/", null);
        root.activate(adapter);

        //
        // Create a file called "README" in the root directory
        //
        FileI file = new FileI(communicator(), "README", root);
        String[] text;
        text = new String[]{ "This file system contains a collection of poetry." };
        try
        {
            file.write(text, null);
        }
        catch(GenericError e)
        {
            System.err.println(e.reason);
        }
        file.activate(adapter);

        //
        // Create a directory called "Coleridge" in the root directory
        //
        DirectoryI coleridge = new DirectoryI(communicator(), "Coleridge", root);
        coleridge.activate(adapter);

        //
        // Create a file called "Kubla_Khan" in the Coleridge directory
        //
        file = new FileI(communicator(), "Kubla_Khan", coleridge);
        text = new String[]{ "In Xanadu did Kubla Khan",
                             "A stately pleasure-dome decree:",
                             "Where Alph, the sacred river, ran",
                             "Through caverns measureless to man",
                             "Down to a sunless sea." };
        try
        {
            file.write(text, null);
        }
        catch(GenericError e)
        {
            System.err.println(e.reason);
        }
        file.activate(adapter);

        //
        // All objects are created, allow client requests now
        //
        adapter.activate();

        //
        // Wait until we are done
        //
        communicator().waitForShutdown();

        if(interrupted())
        {
            System.err.println(appName() + ": terminating");
        }

        return 0;
    }

    public static void
    main(String[] args)
    {
        Server app = new Server();
        System.exit(app.main("Server", args));
    }
}
