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

[assembly: AssemblyTitle("IceValueServer")]
[assembly: AssemblyDescription("Ice value demo server")]
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


	    Ice.ObjectFactory factory = new ObjectFactory();
            communicator().addObjectFactory(factory, Demo.Printer.ice_staticId());

            Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Value");
            Ice.Object @object = new InitialI(adapter);
            adapter.add(@object, communicator().stringToIdentity("initial"));
            adapter.activate();
            communicator().waitForShutdown();
            return 0;
        }
    }

    public static int Main(string[] args)
    {
        App app = new App();
        return app.main(args, "config.server");
    }
}
