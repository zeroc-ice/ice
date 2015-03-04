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

[assembly: AssemblyTitle("IceGridSimpleClient")]
[assembly: AssemblyDescription("IceGrid simple demo client")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    public class App : Ice.Application
    {
        private void menu()
        {
            Console.WriteLine(
                "usage:\n" +
                "t: send greeting\n" +
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

            HelloPrx hello = null;
            try
            {
                hello = HelloPrxHelper.checkedCast(communicator().stringToProxy("hello"));
            }
            catch(Ice.NotRegisteredException)
            {
                IceGrid.QueryPrx query =
                    IceGrid.QueryPrxHelper.checkedCast(communicator().stringToProxy("DemoIceGrid/Query"));
                hello = HelloPrxHelper.checkedCast(query.findObjectByType("::Demo::Hello"));
            }
            if(hello == null)
            {
                Console.WriteLine("couldn't find a `::Demo::Hello' object");
                return 1;
            }

            menu();

            string line = null;
            do
            {
                try
                {
                    Console.Write("==> ");
                    Console.Out.Flush();
                    line = Console.In.ReadLine();
                    if(line == null)
                    {
                        break;
                    }
                    if(line.Equals("t"))
                    {
                        hello.sayHello();
                    }
                    else if(line.Equals("s"))
                    {
                        hello.shutdown();
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
                catch(Ice.LocalException ex)
                {
                    Console.WriteLine(ex);
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
