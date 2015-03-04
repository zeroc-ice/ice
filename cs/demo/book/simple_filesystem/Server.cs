// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Filesystem;
using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceSimpleLifecycleServer")]
[assembly: AssemblyDescription("Ice simple lifecycle demo server")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server
{
    public class App : Ice.Application
    {
        public override int run(string[] args)
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
            string[] text;
            text = new string[]{ "This file system contains a collection of poetry." };
            try
            {
                file.write(text);
            }
            catch(GenericError e)
            {
                Console.Error.WriteLine(e.reason);
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
            text = new string[]{ "In Xanadu did Kubla Khan",
                                 "A stately pleasure-dome decree:",
                                 "Where Alph, the sacred river, ran",
                                 "Through caverns measureless to man",
                                 "Down to a sunless sea." };
            try
            {
                file.write(text);
            }
            catch(GenericError e)
            {
                Console.Error.WriteLine(e.reason);
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
                Console.Error.WriteLine(appName() + ": terminating");
            }

            return 0;
        }
    }

    public static int Main(string[] args)
    {
        App app = new App();
        return app.main(args);
    }
}
