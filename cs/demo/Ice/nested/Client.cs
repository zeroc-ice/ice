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

[assembly: AssemblyTitle("IceNestedClient")]
[assembly: AssemblyDescription("Ice nested demo client")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    public class App : Ice.Application
    {
        public override int run(string[] args)
        {
            if(args.Length > 0)
            {
                Console.Error.WriteLine(appName() + ": too many arguments");
                return 1;
            }

            NestedPrx nested = NestedPrxHelper.checkedCast(communicator().propertyToProxy("Nested.Proxy"));
            if(nested == null)
            {
                Console.Error.WriteLine("invalid proxy");
                return 1;
            }
            
            Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Nested.Client");
            NestedPrx self = 
                NestedPrxHelper.uncheckedCast(adapter.createProxy(communicator().stringToIdentity("nestedClient")));
            adapter.add(new NestedI(self), communicator().stringToIdentity("nestedClient"));
            adapter.activate();
            
            Console.Out.WriteLine("Note: The maximum nesting level is sz * 2, with sz being");
            Console.Out.WriteLine("the maximum number of threads in the server thread pool. If");
            Console.Out.WriteLine("you specify a value higher than that, the application will");
            Console.Out.WriteLine("block or timeout.");
            Console.Out.WriteLine();
            
            string s = null;
            do 
            {
                try
                {
                    Console.Out.Write("enter nesting level or 'x' for exit: ");
                    Console.Out.Flush();
                    s = Console.In.ReadLine();
                    if(s == null)
                    {
                        break;
                    }
                    int level = System.Int32.Parse(s);
                    if(level > 0)
                    {
                        nested.nestedCall(level, self);
                    }
                }
                catch(System.FormatException)
                {
                }
                catch(System.Exception ex)
                {
                    Console.Error.WriteLine(ex);
                }
            }
            while(!s.Equals("x"));
            
            return 0;
        }
    }

    public static int Main(string[] args)
    {
        App app = new App();
        return app.main(args, "config.client");
    }
}
