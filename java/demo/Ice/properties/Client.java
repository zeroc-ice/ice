// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Client extends Ice.Application
{
    class ShutdownHook extends Thread
    {
        public void
        run()
        {
            try
            {
                communicator().destroy();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
            }
        }
    }

    private static void
    menu()
    {
        System.out.println(
            "\n" +
            "usage:\n" +
            "1: set properties (batch 1)\n" +
            "2: set properties (batch 2)\n" +
            "c: show current properties\n" +
            "s: shutdown server\n" +
            "x: exit\n" +
            "?: help\n");
    }

    private static void
    show(Ice.PropertiesAdminPrx admin)
    {
        java.util.Map<String, String> props = admin.getPropertiesForPrefix("Demo");
        System.out.println("Server's current settings:");
        for(java.util.Map.Entry<String, String> e : props.entrySet())
        {
            System.out.println("  " + e.getKey() + "=" + e.getValue());
        }
    }

    public int
    run(String[] args)
    {
        if(args.length > 0)
        {
            System.err.println(appName() + ": too many arguments");
            return 1;
        }

        //
        // Since this is an interactive demo we want to clear the
        // Application installed interrupt callback and install our
        // own shutdown hook.
        //
        setInterruptHook(new ShutdownHook());

        PropsPrx props = PropsPrxHelper.checkedCast(communicator().propertyToProxy("Props.Proxy"));
        if(props == null)
        {
            System.err.println("invalid proxy");
            return 1;
        }

        Ice.PropertiesAdminPrx admin =
            Ice.PropertiesAdminPrxHelper.checkedCast(communicator().propertyToProxy("Admin.Proxy"));

        java.util.Map<String, String> batch1 = new java.util.HashMap<String, String>();
        batch1.put("Demo.Prop1", "1");
        batch1.put("Demo.Prop2", "2");
        batch1.put("Demo.Prop3", "3");

        java.util.Map<String, String> batch2 = new java.util.HashMap<String, String>();
        batch2.put("Demo.Prop1", "10");
        batch2.put("Demo.Prop2", ""); // An empty value removes this property
        batch2.put("Demo.Prop3", "30");

        show(admin);
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
                if(line.equals("1") || line.equals("2"))
                {
                    java.util.Map<String, String> dict = line.equals("1") ? batch1 : batch2;
                    System.out.println("Sending:");
                    for(java.util.Map.Entry<String, String> e : dict.entrySet())
                    {
                        if(e.getKey().startsWith("Demo"))
                        {
                            System.out.println("  " + e.getKey() + "=" + e.getValue());
                        }
                    }
                    System.out.println();

                    admin.setProperties(dict);

                    System.out.println("Changes:");
                    java.util.Map<String, String> changes = props.getChanges();
                    if(changes.isEmpty())
                    {
                        System.out.println("  None.");
                    }
                    else
                    {
                        for(java.util.Map.Entry<String, String> e : changes.entrySet())
                        {
                            System.out.print("  " + e.getKey());
                            if(e.getValue().length() == 0)
                            {
                                System.out.println(" was removed");
                            }
                            else
                            {
                                System.out.println(" is now " + e.getValue());
                            }
                        }
                    }
                }
                else if(line.equals("c"))
                {
                    show(admin);
                }
                else if(line.equals("s"))
                {
                    props.shutdown();
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
        Client app = new Client();
        int status = app.main("Client", args, "config.client");
        System.exit(status);
    }
}
