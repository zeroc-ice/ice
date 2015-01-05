// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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
        @Override
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

    @Override
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

        //
        // Get the hello proxy. We configure the proxy to not cache the
        // server connection with the proxy and to disable the locator
        // cache. With this configuration, the IceGrid locator will be
        // queried for each invocation on the proxy and the invocation
        // will be sent over the server connection matching the returned
        // endpoints.
        //
        Ice.ObjectPrx obj = communicator().stringToProxy("hello");
        obj = obj.ice_connectionCached(false);
        obj = obj.ice_locatorCacheTimeout(0);

        HelloPrx hello = HelloPrxHelper.checkedCast(obj);
        if(hello == null)
        {
            System.err.println("invalid proxy");
            return 1;
        }

        while(true)
        {
            System.out.print("enter the number of iterations: ");
            System.out.flush();

            try
            {
                java.io.BufferedReader in = new java.io.BufferedReader(new java.io.InputStreamReader(System.in));
                String line = in.readLine();
                if(line == null || line.equals("x"))
                {
                    break;
                }

                int count = Integer.parseInt(line);

                System.out.print("enter the delay between each greetings (in ms): ");
                System.out.flush();
                line = in.readLine();
                if(line == null || line.equals("x"))
                {
                    break;
                }
                int delay = Integer.parseInt(line);

                if(delay < 0)
                {
                    delay = 500; // 500 milli-seconds
                }
                
                for(int i = 0; i < count; i++)
                {
                    System.out.println(hello.getGreeting());
                    try
                    {
                        Thread.currentThread();
                        Thread.sleep(delay);
                    }
                    catch(InterruptedException ex1)
                    {
                    }
                }
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
            }
            catch(NumberFormatException ex)
            {
                System.out.println("please specify a valid integer value");
            }
            catch(java.io.IOException ex)
            {
                ex.printStackTrace();
            }
        }
            
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
