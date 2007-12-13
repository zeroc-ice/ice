// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Demo;

public class Client : Ice.Application
{
    private static void menu()
    {
        Console.Write(
            "usage:\n" +
            "t: send multicast message\n" +
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

        HelloPrx proxy = HelloPrxHelper.uncheckedCast(communicator().propertyToProxy("Hello.Proxy").ice_datagram());

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
                    proxy.sayHello();
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

    public static void Main(string[] args)
    {
        Client app = new Client();
        int status = app.main(args, "config.client");
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}
