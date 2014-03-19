// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceGridSimpleServer")]
[assembly: AssemblyDescription("IceGrid simple demo server")]
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

            Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Hello");
            Ice.Properties properties = communicator().getProperties();
            Ice.Identity id = communicator().stringToIdentity(properties.getProperty("Identity"));
            adapter.add(new HelloI(properties.getProperty("Ice.ProgramName")), id);
            adapter.activate();
            communicator().waitForShutdown();
            return 0;
        }
    }

    static public int Main(string[] args)
    {
        App app = new App();
        return app.main(args);
    }
}
