// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;
using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceCallbackClient")]
[assembly: AssemblyDescription("Ice callback demo client")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    public class App : Ice.Application
    {
        private static void menu()
        {
            Console.Out.Write("usage:\n"
                              + "t: send callback\n"
                              + "s: shutdown server\n"
                              + "x: exit\n"
                              + "?: help\n");
        }
        
        public override int run(string[] args)
        {
            if(args.Length > 0)
            {
                Console.Error.WriteLine(appName() + ": too many arguments");
                return 1;
            }

            CallbackSenderPrx sender = CallbackSenderPrxHelper.checkedCast(
                communicator().propertyToProxy("CallbackSender.Proxy").
                    ice_twoway().ice_timeout(-1).ice_secure(false));
            if(sender == null)
            {
                Console.Error.WriteLine("invalid proxy");
                return 1;
            }
            
            Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Callback.Client");
            adapter.add(new CallbackReceiverI(), communicator().stringToIdentity("callbackReceiver"));
            adapter.activate();
            
            CallbackReceiverPrx receiver = CallbackReceiverPrxHelper.uncheckedCast(
                                           adapter.createProxy(communicator().stringToIdentity("callbackReceiver")));

            menu();
            
            string line = null;
            do 
            {
                try
                {
                    Console.Out.Write("==> ");
                    Console.Out.Flush();
                    line = Console.In.ReadLine();
                    if(line == null)
                    {
                        break;
                    }
                    if(line.Equals("t"))
                    {
                        sender.initiateCallback(receiver);
                    }
                    else if(line.Equals("s"))
                    {
                        sender.shutdown();
                    }
                    else if(line.Equals("x"))
                    {
                        // Nothing to do
                    }
                    else if(line.Equals("?"))
                    {
                        menu();
                    }
                    else
                    {
                        Console.Out.WriteLine("unknown command `" + line + "'");
                        menu();
                    }
                }
                catch(System.Exception ex)
                {
                    Console.Error.WriteLine(ex);
                }
            }
            while(!line.Equals("x"));
            
            return 0;
        }
    }

    public static int Main(string[] args)
    {
        App app = new App();
        return app.main(args, "config.client");
    }
}
