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
using System.Collections.Generic;

public class Client : Ice.Application
{
    private static void menu()
    {
        Console.Out.WriteLine(
            "\n" +
            "usage:\n" +
            "1: set properties (batch 1)\n" +
            "2: set properties (batch 2)\n" +
            "c: show current properties\n" +
            "s: shutdown server\n" +
            "x: exit\n" +
            "?: help\n");
    }

    private static void show(Ice.PropertiesAdminPrx admin)
    {
        Dictionary<string, string> props = admin.getPropertiesForPrefix("Demo");
        Console.Out.WriteLine("Server's current settings:");
        foreach(KeyValuePair<string, string> e in props)
        {
            Console.Out.WriteLine("  " + e.Key + "=" + e.Value);
        }
    }

    override public int run(string[] args)
    {
        if(args.Length > 0)
        {
            Console.Error.WriteLine(appName() + ": too many arguments");
            return 1;
        }

        PropsPrx props = PropsPrxHelper.checkedCast(communicator().propertyToProxy("Props.Proxy"));
        if(props == null)
        {
            Console.Error.WriteLine("invalid proxy");
            return 1;
        }

        Ice.PropertiesAdminPrx admin =
            Ice.PropertiesAdminPrxHelper.checkedCast(communicator().propertyToProxy("Admin.Proxy"));

        Dictionary<string, string> batch1 = new Dictionary<string, string>();
        batch1.Add("Demo.Prop1", "1");
        batch1.Add("Demo.Prop2", "2");
        batch1.Add("Demo.Prop3", "3");

        Dictionary<string, string> batch2 = new Dictionary<string, string>();
        batch2.Add("Demo.Prop1", "10");
        batch2.Add("Demo.Prop2", ""); // An empty value removes this property
        batch2.Add("Demo.Prop3", "30");

        show(admin);
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
                if(line.Equals("1") || line.Equals("2"))
                {
                    Dictionary<string, string> dict = line.Equals("1") ? batch1 : batch2;
                    Console.Out.WriteLine("Sending:");
                    foreach(KeyValuePair<string, string> e in dict)
                    {
                        if(e.Key.StartsWith("Demo"))
                        {
                            Console.Out.WriteLine("  " + e.Key + "=" + e.Value);
                        }
                    }
                    Console.Out.WriteLine();

                    admin.setProperties(dict);

                    Console.Out.WriteLine("Changes:");
                    Dictionary<string, string> changes = props.getChanges();
                    if(changes.Count == 0)
                    {
                        Console.Out.WriteLine("  None.");
                    }
                    else
                    {
                        foreach(KeyValuePair<string, string> e in changes)
                        {
                            Console.Out.Write("  " + e.Key);
                            if(e.Value.Length == 0)
                            {
                                Console.Out.WriteLine(" was removed");
                            }
                            else
                            {
                                Console.Out.WriteLine(" is now " + e.Value);
                            }
                        }
                    }
                }
                else if(line.Equals("c"))
                {
                    show(admin);
                }
                else if(line.Equals("s"))
                {
                    props.shutdown();
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
            catch(System.Exception ex)
            {
                Console.Error.WriteLine(ex);
            }
        }
        while(!line.Equals("x"));

        return 0;
    }

    public static int Main(string[] args)
    {
        Client app = new Client();
        return app.main(args, "config.client");
    }
}
