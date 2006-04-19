// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Client extends Ice.Application
{
    public int
    run(String[] args)
    {
        Ice.Properties properties = communicator().getProperties();
        final String proxyProperty = "Nested.Client.NestedServer";
        String proxy = properties.getProperty(proxyProperty);
        if(proxy.length() == 0)
        {
            System.err.println("property `" + proxyProperty + "' not set");
            return 1;
        }

        NestedPrx nested = NestedPrxHelper.checkedCast(communicator().stringToProxy(proxy));
        if(nested == null)
        {
            System.err.println("invalid proxy");
            return 1;
        }

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Nested.Client");
        NestedPrx self = NestedPrxHelper.uncheckedCast(adapter.createProxy(Ice.Util.stringToIdentity("nestedClient")));
        adapter.add(new NestedI(self), Ice.Util.stringToIdentity("nestedClient"));
        adapter.activate();

        System.out.println("Note: The maximum nesting level is sz * 2, with sz being");
        System.out.println("the maximum number of threads in the server thread pool. If");
        System.out.println("you specify a value higher than that, the application will");
        System.out.println("block or timeout.");
        System.out.println();

        java.io.BufferedReader in = new java.io.BufferedReader(new java.io.InputStreamReader(System.in));

        String s = null;
        do
        {
            try
            {
                System.out.print("enter nesting level or 'x' for exit: ");
                System.out.flush();
                s = in.readLine();
                if(s == null)
                {
                    break;
                }
                int level = Integer.parseInt(s);
                if(level > 0)
                {
                    nested.nestedCall(level, self);
                }
            }
            catch(NumberFormatException ex)
            {
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
        while(!s.equals("x"));

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
