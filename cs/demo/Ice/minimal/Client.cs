// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Demo;

public class Client
{
    private static void menu()
    {
        Console.WriteLine(
	    "usage:\n" +
	    "h: send greeting\n" +
	    "x: exit\n" +
	    "?: help\n");
    }
    
    public static int run(string[] args, Ice.Communicator communicator)
    {
        Ice.Properties properties = communicator.getProperties();
        string proxyProperty = "Hello.Proxy";
        string proxy = properties.getProperty(proxyProperty);
        if(proxy.Length == 0)
        {
            Console.Error.WriteLine("property `" + proxyProperty + "' not set");
            return 1;
        }
        
        HelloPrx hello = HelloPrxHelper.checkedCast(
	    communicator.stringToProxy(proxy).ice_twoway().ice_timeout(-1).ice_secure(false));
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
                if(line.Equals("h"))
                {
                    hello.sayHello();
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

    public static void Main(string[] args)
    {
	int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            Ice.Properties properties = Ice.Util.createProperties();
            properties.load("config");
            communicator = Ice.Util.initializeWithProperties(ref args, properties);
            status = run(args, communicator);
        }
        catch(System.Exception ex)
        {
            Console.Error.WriteLine(ex);
            status = 1;
        }

        if(communicator != null)
        {
            try
            {
                communicator.destroy();
            }
            catch(System.Exception ex)
            {
                Console.Error.WriteLine(ex);
                status = 1;
            }
        }

        System.Environment.Exit(status);
    }
}
