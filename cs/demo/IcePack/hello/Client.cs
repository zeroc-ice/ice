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
            "c: create a hello object\n" +
            "d: destroy the current hello object\n" +
            "s: set the current hello object\n" +
            "r: set the current hello object to a random hello object\n" +
            "S: show the name of the current hello object\n" +
            "t: send greeting\n" +
            "x: exit\n" +
            "?: help\n");
    }

    private static int run(string[] args, Ice.Communicator communicator)
    {
        IcePack.QueryPrx query = IcePack.QueryPrxHelper.checkedCast(communicator.stringToProxy("IcePack/Query"));

        //
        // Get an object implementing the HelloFactory interface.
        //
        HelloFactoryPrx factory = HelloFactoryPrxHelper.checkedCast(query.findObjectByType("::Demo::HelloFactory"));

        //
        // By default we create a Hello object named 'Foo'.
        //
        HelloPrx hello;
        try
        {
            hello = factory.find("Foo");
        }
        catch(NameNotExistException)
        {
            hello = factory.create("Foo");
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
                else if(line.Equals("c"))
                {
                    Console.Write("name: ");
                    Console.Out.Flush();
                    string name = Console.In.ReadLine();
                    if(name != null && name.Length > 0)
                    {
                        try
                        {
                            hello = factory.find(name);
                            Console.WriteLine("Hello object named '" + name + "' already exists");
                        }
                        catch(NameNotExistException)
                        {
                            factory = HelloFactoryPrxHelper.checkedCast(query.findObjectByType("::Demo::HelloFactory"));
                            hello = factory.create(name);
                        }
                    }
                }
                else if(line.Equals("d"))
                {
                    if(Ice.Util.identityToString(hello.ice_getIdentity()).Equals("Foo"))
                    {
                        Console.WriteLine("Can't delete the default Hello object named 'Foo'");
                    }
                    else
                    {
                        hello.destroy();

                        try
                        {
                            hello = factory.find("Foo");
                        }
                        catch(NameNotExistException)
                        {
                            hello = factory.create("Foo");
                        }
                    }
                }
                else if(line.Equals("s"))
                {
                    Console.Write("name: ");
                    Console.Out.Flush();
                    string name = Console.In.ReadLine();
                    if(name != null && name.Length > 0)
                    {
                        try
                        {
                            hello = HelloPrxHelper.checkedCast(factory.find(name));
                        }
                        catch(NameNotExistException)
                        {
                            Console.WriteLine("This name doesn't exist");
                        }
                    }
                }
                else if(line.Equals("r"))
                {
                    hello = HelloPrxHelper.checkedCast(query.findObjectByType("::Demo::Hello"));
                }
                else if(line.Equals("S"))
                {
                    Console.WriteLine(Ice.Util.identityToString(hello.ice_getIdentity()));
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
                Console.Error.WriteLine(ex);
            }
        }
        while(!line.Equals("x"));

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
        catch(Exception ex)
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
            catch(Ice.LocalException ex)
            {
                Console.Error.WriteLine(ex);
                status = 1;
            }
        }

        Environment.Exit(status);
    }
}
