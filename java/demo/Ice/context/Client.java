// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
            "usage:\n" +
            "1: call with no request context\n" +
            "2: call with explicit request context\n" +
            "3: call with per-proxy request context\n" +
            "4: call with implicit request context\n" +
            "s: shutdown server\n" +
            "x: exit\n" +
            "?: help\n");
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

        ContextPrx proxy = ContextPrxHelper.checkedCast(communicator().propertyToProxy("Context.Proxy"));
        if(proxy == null)
        {
            System.err.println("invalid proxy");
            return 1;
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
                if(line.equals("1"))
                {
                    proxy.call();
                }
                else if(line.equals("2"))
                {
                    java.util.Map<String, String> ctx = new java.util.HashMap<String, String>();
                    ctx.put("type", "Explicit");
                    proxy.call(ctx);
                }
                else if(line.equals("3"))
                {
                    java.util.Map<String, String> ctx = new java.util.HashMap<String, String>();
                    ctx.put("type", "Per-Proxy");
                    ContextPrx proxy2 = ContextPrxHelper.uncheckedCast(proxy.ice_context(ctx));
                    proxy2.call();
                }
                else if(line.equals("4"))
                {
                    Ice.ImplicitContext ic = communicator().getImplicitContext();
                    java.util.Map<String, String> ctx = new java.util.HashMap<String, String>();
                    ctx.put("type", "Implicit");
                    ic.setContext(ctx);
                    proxy.call();
                    ic.setContext(new java.util.HashMap<String, String>());
                }
                else if(line.equals("s"))
                {
                    proxy.shutdown();
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

