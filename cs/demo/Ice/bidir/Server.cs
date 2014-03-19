// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;
using System;
using System.Threading;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceBidirServer")]
[assembly: AssemblyDescription("Ice bidir demo server")]
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

            Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Callback.Server");
            CallbackSenderI sender = new CallbackSenderI(communicator());
            adapter.add(sender, communicator().stringToIdentity("sender"));
            adapter.activate();

            Thread t = new Thread(new ThreadStart(sender.Run));
            t.Start();

            try
            {
                communicator().waitForShutdown();
            }
            finally
            {
                sender.destroy();
                t.Join();
            }

            return 0;
        }
    }

    public static int Main(string[] args)
    {
        App app = new App();
        return app.main(args, "config.server");
    }
}
