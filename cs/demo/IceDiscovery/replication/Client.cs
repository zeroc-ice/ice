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

[assembly: AssemblyTitle("IceDiscoveryReplicationClient")]
[assembly: AssemblyDescription("IceDiscovery replication demo client")]
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

            //
            // Get the hello proxy. We configure the proxy to not cache the
            // server connection with the proxy and to disable the locator
            // cache. With this configuration, the IceGrid locator will be
            // queried for each invocation on the proxy and the invocation
            // will be sent over the server connection matching the returned
            // endpoints.
            //
            Ice.ObjectPrx obj = communicator().stringToProxy("hello");
            obj = obj.ice_connectionCached(false);
            obj = obj.ice_locatorCacheTimeout(0);

            HelloPrx hello = HelloPrxHelper.checkedCast(obj);
            if(hello == null)
            {
                Console.Error.WriteLine("invalid proxy");
                return 1;
            }

            while(true)
            {
                Console.Out.Write("enter the number of iterations: ");
                Console.Out.Flush();
                string line = Console.In.ReadLine();
                if(line == null || line.Equals("x"))
                {
                    break;
                }
                int count = Convert.ToInt32(line);

                Console.Out.Write("enter the delay between each greetings (in ms): ");
                Console.Out.Flush();
                line = Console.In.ReadLine();
                if(line == null || line.Equals("x"))
                {
                    break;
                }
                int delay = Convert.ToInt32(line);

                if(delay < 0)
                {
                    delay = 500; // 500 milli-seconds
                }
        
                for(int i = 0; i < count; i++)
                {
                    Console.Out.WriteLine(hello.getGreeting());
                    System.Threading.Thread.Sleep(delay);
                }
            }
            
            return 0;
        }
    }

    public static int Main(string[] args)
    {
        App app = new App();
        return app.main(args, "config.client");
    }
}
