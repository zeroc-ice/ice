// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;
using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceCallbackServer")]
[assembly: AssemblyDescription("Ice callback demo server")]
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
            adapter.add(new CallbackSenderI(), communicator().stringToIdentity("callbackSender"));
            adapter.activate();
            communicator().waitForShutdown();
            return 0;
        }
    }

    public static void Main(string[] args)
    {
        App app = new App();
        int status = app.main(args, "config.server");
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}
