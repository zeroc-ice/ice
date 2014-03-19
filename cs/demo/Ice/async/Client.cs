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

[assembly: AssemblyTitle("IceAsyncClient")]
[assembly: AssemblyDescription("Ice async demo client")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    public class App : Ice.Application
    {
        public void success()
        {
        }

        public void exception(Ice.Exception ex)
        {
            if(ex is RequestCanceledException)
            {
                Console.Error.WriteLine("RequestCanceledException");
            }
            else
            {
                Console.Error.WriteLine("sayHello AMI call failed:");
                Console.Error.WriteLine(ex);
            }
        }

        private static void menu()
        {
            Console.Out.WriteLine(
                "usage:\n" +
                "i: send immediate greeting\n" +
                "d: send delayed greeting\n" +
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

            HelloPrx hello = HelloPrxHelper.checkedCast(communicator().propertyToProxy("Hello.Proxy"));
            if(hello == null)
            {
                Console.Error.WriteLine("invalid proxy");
                return 1;
            }

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
                    if(line.Equals("i"))
                    {
                        hello.sayHello(0);
                    }
                    else if(line.Equals("d"))
                    {
                        hello.begin_sayHello(5000).whenCompleted(success, exception);
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
                        Console.Out.WriteLine("unknown command `" + line + "'");
                        menu();
                    }
                }
                catch(Ice.Exception ex)
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
