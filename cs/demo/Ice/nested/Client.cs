// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Demo;

public class Client : Ice.Application
{
    public override int run(string[] args)
    {
        Ice.Properties properties = communicator().getProperties();
        string proxyProperty = "Nested.Client.NestedServer";
        string proxy = properties.getProperty(proxyProperty);
        if(proxy.Length == 0)
        {
            Console.Error.WriteLine("property `" + proxyProperty + "' not set");
            return 1;
        }
        
        NestedPrx nested = NestedPrxHelper.checkedCast(communicator().stringToProxy(proxy));
        if(nested == null)
        {
            Console.Error.WriteLine("invalid proxy");
            return 1;
        }
        
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Nested.Client");
        NestedPrx self = NestedPrxHelper.uncheckedCast(adapter.createProxy(Ice.Util.stringToIdentity("nestedClient")));
        adapter.add(new NestedI(self), Ice.Util.stringToIdentity("nestedClient"));
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

    public static void Main(string[] args)
    {
        Client app = new Client();
        int status = app.main(args, "config");
        System.Environment.Exit(status);
    }
}
