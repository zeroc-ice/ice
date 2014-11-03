// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using FilesystemI;
using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceLifecycleServer")]
[assembly: AssemblyDescription("Ice lifecycle demo server")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server
{
    class App : Ice.Application
    {
        public override int run(string[] args)
        {
            //
            // Terminate cleanly on receipt of a signal.
            //
            shutdownOnInterrupt();

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
                System.Console.Error.WriteLine(appName() + ": received signal, shutting down");
            }

            return 0;
        }
    }

    static public int Main(string[] args)
    {
        App app = new App();
        return app.main(args);
    }
}
