// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;

using System;
using System.Reflection;
using System.Collections.Generic;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceContextClient")]
[assembly: AssemblyDescription("Ice context demo client")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    public class App : Ice.Application
    {
        private static void menu()
        {
            Console.Write(
                "usage:\n" +
                "1: call with no request context\n" +
                "2: call with explicit request context\n" +
                "3: call with per-proxy request context\n" +
                "4: call with implicit request context\n" +
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

            ContextPrx proxy = ContextPrxHelper.checkedCast(communicator().propertyToProxy("Context.Proxy"));
            if(proxy == null)
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
                    if(line.Equals("1"))
                    {

                        proxy.call();
                    }
                    else if(line.Equals("2"))
                    {
                        Dictionary<string, string> ctx = new Dictionary<string, string>();
                        ctx["type"] = "Explicit";
                        proxy.call(ctx);
                    }
                    else if(line.Equals("3"))
                    {
                        Dictionary<string, string> ctx = new Dictionary<string, string>();
                        ctx["type"] = "Per-Proxy";
                        ContextPrx proxy2 = ContextPrxHelper.uncheckedCast(proxy.ice_context(ctx));
                        proxy2.call();
                    }
                    else if(line.Equals("4"))
                    {
                        Ice.ImplicitContext ic = communicator().getImplicitContext();
                        Dictionary<string, string> ctx = new Dictionary<string, string>();
                        ctx["type"] = "Implicit";
                        ic.setContext(ctx);
                        proxy.call();
                        ic.setContext(new Dictionary<string, string>());
                    }
                    else if(line.Equals("s"))
                    {
                        proxy.shutdown();
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
