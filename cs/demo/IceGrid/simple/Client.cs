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
