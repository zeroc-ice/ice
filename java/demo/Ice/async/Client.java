// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

    public class AMI_Hello_sayHelloI extends AMI_Hello_sayHello
    {
        public void ice_response()
        {
        }

        public void ice_exception(Ice.LocalException ex)
        {
            System.err.println("sayHello AMI call failed:");
            ex.printStackTrace();
        }

        public void ice_exception(Ice.UserException ex)
        {
            if(ex instanceof Demo.RequestCanceledException)
            {
                System.out.println("Request canceled");
            }
            else
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
            "i: send immediate greeting\n" +
            "d: send delayed greeting\n" +
            "s: shutdown the server\n" +
            "x: exit\n" +
            "?: help\n");
    }

    public int
    run(String[] args)
    {
        //
        // Since this is an interactive demo we want to clear the
        // Application installed interrupt callback and install our
        // own shutdown hook.
        //
        setInterruptHook(new ShutdownHook());

        HelloPrx hello = HelloPrxHelper.checkedCast(communicator().propertyToProxy("Hello.Proxy"));
        if(hello == null)
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
                if(line.equals("i"))
                {
                    hello.sayHello(0);
                }
                else if(line.equals("d"))
                {
                    hello.sayHello_async(new AMI_Hello_sayHelloI(), 5000);
                }
                else if(line.equals("s"))
                {
                    hello.shutdown();
                }
                else if(line.equals("x"))
                {
                    // Nothing to do
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
            catch(Ice.UserException ex)
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
