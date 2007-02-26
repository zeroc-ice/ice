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
    public class AMI_Hello_sayHelloI : AMI_Hello_sayHello
    {
        public override void ice_response()
        {
        }

        public override void ice_exception(Ice.Exception ex)
        {
            if(ex is RequestCanceledException)
            {
                Console.Error.WriteLine("Request canceled");
            }
            else
            {
                Console.Error.WriteLine("sayHello AMI call failed:");
                Console.Error.WriteLine(ex);
            }
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
                    hello.sayHello_async(new AMI_Hello_sayHelloI(), 5000);
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
