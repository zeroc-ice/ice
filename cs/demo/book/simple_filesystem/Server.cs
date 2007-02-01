// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Filesystem;
using System;

public class Server : Ice.Application
{
    public override int run(string[] args)
    {
        // Terminate cleanly on receipt of a signal
        //
        shutdownOnInterrupt();

        // Create an object adapter (stored in the _adapter
        // static members)
        //
        Ice.ObjectAdapter adapter = communicator().createObjectAdapterWithEndpoints(
                                        "SimpleFilesystem", "default -p 10000");
        DirectoryI._adapter = adapter;
        FileI._adapter = adapter;

        // Create the root directory (with name "/" and no parent)
        //
        DirectoryI root = new DirectoryI("/", null);

        // Create a file called "README" in the root directory
        //
        File file = new FileI("README", root);
        string[] text;
        text = new string[]{ "This file system contains a collection of poetry." };
        try {
            file.write(text);
        } catch (GenericError e) {
            Console.Error.WriteLine(e.reason);
        }

        // Create a directory called "Coleridge" in the root directory
        //
        DirectoryI coleridge = new DirectoryI("Coleridge", root);

        // Create a file called "Kubla_Khan" in the Coleridge directory
        //
        file = new FileI("Kubla_Khan", coleridge);
        text = new string[]{ "In Xanadu did Kubla Khan",
                             "A stately pleasure-dome decree:",
                             "Where Alph, the sacred river, ran",
                             "Through caverns measureless to man",
                             "Down to a sunless sea." };
        try {
            file.write(text);
        } catch (GenericError e) {
            Console.Error.WriteLine(e.reason);
        }

        // All objects are created, allow client requests now
        //
        adapter.activate();

        // Wait until we are done
        //
        communicator().waitForShutdown();

        if (interrupted())
            Console.Error.WriteLine(appName() + ": terminating");

        return 0;
    }

    public static void Main(string[] args)
    {
        Server app = new Server();
        int status = app.main(args);
        if(status != 0)
        {
            Environment.Exit(status);
        }
    }
}
