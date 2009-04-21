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

[assembly: AssemblyTitle("IcePluginClient")]
[assembly: AssemblyDescription("Ice plugin demo client")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    public class App : Ice.Application
    {
        private static void menu()
        {
            Console.Write(
                "usage:\n" +
                "t: send greeting as twoway\n" +
                "o: send greeting as oneway\n" +
                "O: send greeting as batch oneway\n" +
                "d: send greeting as datagram\n" +
                "D: send greeting as batch datagram\n" +
                "f: flush all batch requests\n" +
                "s: shutdown server\n" +
                "x: exit\n" +
                "?: help\n");
        }

        public override int run(string[] args)
        {
            if(args.Length > 0)
            {
                Console.Error.WriteLine(appName() + ": too many arguments");
                return 1;
            }

            HelloPrx twoway = HelloPrxHelper.checkedCast(communicator().propertyToProxy("Hello.Proxy"));
            if(twoway == null)
            {
                Console.Error.WriteLine("invalid proxy");
                return 1;
            }
            HelloPrx oneway = (HelloPrx)twoway.ice_oneway();
            HelloPrx batchOneway = (HelloPrx)twoway.ice_batchOneway();
            HelloPrx datagram = (HelloPrx)twoway.ice_datagram();
            HelloPrx batchDatagram =(HelloPrx)twoway.ice_batchDatagram();

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
                        twoway.sayHello();
                    }
                    else if(line.Equals("o"))
                    {
                        oneway.sayHello();
                    }
                    else if(line.Equals("O"))
                    {
                        batchOneway.sayHello();
                    }
                    else if(line.Equals("d"))
                    {
                        datagram.sayHello();
                    }
                    else if(line.Equals("D"))
                    {
                        batchDatagram.sayHello();
                    }
                    else if(line.Equals("f"))
                    {
                        communicator().flushBatchRequests();
                    }
                    else if(line.Equals("s"))
                    {
                        twoway.shutdown();
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
                        Console.WriteLine("unknown command `" + line + "'");
                        menu();
                    }
                }
                catch(System.Exception ex)
                {
                    Console.Error.WriteLine(ex);
                }
            }
            while (!line.Equals("x"));
            
            return 0;
        }
    }

    public static void Main(string[] args)
    {
        App app = new App();
        int status = app.main(args, "config.client");
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}
