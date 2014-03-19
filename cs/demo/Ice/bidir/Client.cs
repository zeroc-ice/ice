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
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceBidirClient")]
[assembly: AssemblyDescription("Ice bidir demo client")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
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

            CallbackSenderPrx server = CallbackSenderPrxHelper.checkedCast(
                communicator().propertyToProxy("CallbackSender.Proxy"));
            if(server == null)
            {
                System.Console.Error.WriteLine("invalid proxy");
                return 1;
            }

            Ice.ObjectAdapter adapter = communicator().createObjectAdapter("");
            Ice.Identity ident = new Ice.Identity();
            ident.name = Guid.NewGuid().ToString();
            ident.category = "";
            adapter.add(new CallbackReceiverI(), ident);
            adapter.activate();
            server.ice_getConnection().setAdapter(adapter);
            server.addClient(ident);
            communicator().waitForShutdown();

            return 0;
        }
    }

    public static int Main(string[] args)
    {
        App app = new App();
        return app.main(args, "config.client");
    }
}
