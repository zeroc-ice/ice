// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Client
{
    private static void
    menu()
    {
        System.out.println(
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

    private static int
    run(String[] args, Ice.Communicator communicator)
        throws Ice.UserException
    {
        Ice.Properties properties = communicator.getProperties();

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
        catch(NameNotExistException ex)
        {
            hello = factory.create("Foo");
        }

        menu();

        java.io.BufferedReader in = new java.io.BufferedReader(new java.io.InputStreamReader(System.in));

        String line = null;
        do
        {
            try
            {
                System.out.print("==> ");
                System.out.flush();
                line = in.readLine();
                if(line == null)
                {
                    break;
                }
                if(line.equals("t"))
                {
                    hello.sayHello();
                }
                else if(line.equals("c"))
                {
                    System.out.print("name: ");
                    System.out.flush();
                    String name = in.readLine();
                    if(name != null && name.length() > 0)
                    {
                        try
                        {
                            hello = factory.find(name);
                            System.out.println("Hello object named '" + name + "' already exists");
                        }
                        catch(NameNotExistException ex)
                        {
                            factory = HelloFactoryPrxHelper.checkedCast(query.findObjectByType("::Demo::HelloFactory"));
                            hello = factory.create(name);
                        }
                    }
                }
                else if(line.equals("d"))
                {
                    if(Ice.Util.identityToString(hello.ice_getIdentity()).equals("Foo"))
                    {
                        System.out.println("Can't delete the default Hello object named 'Foo'");
                    }
                    else
                    {
                        hello.destroy();

                        try
                        {
                            hello = factory.find("Foo");
                        }
                        catch(NameNotExistException ex)
                        {
                            hello = factory.create("Foo");
                        }
                    }
                }
                else if(line.equals("s"))
                {
                    System.out.print("name: ");
                    System.out.flush();
                    String name = in.readLine();
                    if(name != null && name.length() > 0)
                    {
                        try
                        {
                            hello = HelloPrxHelper.checkedCast(factory.find(name));
                        }
                        catch(NameNotExistException ex)
                        {
                            System.out.println("This name doesn't exist");
                        }
                    }
                }
                else if(line.equals("r"))
                {
                    hello = HelloPrxHelper.checkedCast(query.findObjectByType("::Demo::Hello"));
                }
                else if(line.equals("S"))
                {
                    System.out.println(Ice.Util.identityToString(hello.ice_getIdentity()));
                }
                else if(line.equals("x"))
                {
                    // Nothing to do
                }
                else if(line.equals("?"))
                {
                    menu();
                }
                else
                {
                    System.out.println("unknown command `" + line + "'");
                    menu();
                }
            }
            catch(java.io.IOException ex)
            {
                ex.printStackTrace();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
            }
        }
        while(!line.equals("x"));

        return 0;
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            Ice.Properties properties = Ice.Util.createProperties();
            properties.load("config");
            communicator = Ice.Util.initializeWithProperties(args, properties);
            status = run(args, communicator);
        }
        catch(Exception ex)
        {
            ex.printStackTrace();
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
                ex.printStackTrace();
                status = 1;
            }
        }

        System.exit(status);
    }
}
