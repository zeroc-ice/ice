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

[assembly: AssemblyTitle("IceSerializeClient")]
[assembly: AssemblyDescription("Ice serialize demo client")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    public class App : Ice.Application
    {
        private static void menu()
        {
            Console.Write(
                "usage:\n" +
                "g: send greeting\n" +
                "t: toggle null greeting\n" +
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

            GreetPrx greet = GreetPrxHelper.checkedCast(communicator().propertyToProxy("Greet.Proxy"));
            if(greet == null)
            {
                Console.Error.WriteLine("invalid proxy");
                return 1;
            }

            MyGreeting greeting = new MyGreeting();
            greeting.text = "Hello there!";
            MyGreeting nullGreeting = null;

            bool sendNull = false;

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
                    if(line.Equals("g"))
                    {
                        if(sendNull)
                        {
                            greet.sendGreeting(nullGreeting);
                        }
                        else
                        {
                            greet.sendGreeting(greeting);
                        }
                    }
                    else if(line.Equals("t"))
                    {
                        sendNull = !sendNull;
                    }
                    else if(line.Equals("s"))
                    {
                        greet.shutdown();
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

    public static int Main(string[] args)
    {
        App app = new App();
        return app.main(args, "config.client");
    }
}
