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
            "t: send greeting as twoway\n" +
            "o: send greeting as oneway\n" +
            "O: send greeting as batch oneway\n" +
            "d: send greeting as datagram\n" +
            "D: send greeting as batch datagram\n" +
            "f: flush all batch requests\n" +
            "T: set a timeout\n" +
            "P: set a server delay\n" +
            "S: switch secure mode on/off\n" +
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

        HelloPrx twoway = HelloPrxHelper.checkedCast(
            communicator().propertyToProxy("Hello.Proxy").ice_twoway().ice_timeout(-1).ice_secure(false));
        if(twoway == null)
        {
            System.err.println("invalid proxy");
            return 1;
        }
        HelloPrx oneway = (HelloPrx)twoway.ice_oneway();
        HelloPrx batchOneway = (HelloPrx)twoway.ice_batchOneway();
        HelloPrx datagram = (HelloPrx)twoway.ice_datagram();
        HelloPrx batchDatagram = (HelloPrx)twoway.ice_batchDatagram();

        boolean secure = false;
        int timeout = -1;
        int delay = 0;

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
                    twoway.sayHello(delay);
                }
                else if(line.equals("o"))
                {
                    oneway.sayHello(delay);
                }
                else if(line.equals("O"))
                {
                    batchOneway.sayHello(delay);
                }
                else if(line.equals("d"))
                {
                    if(secure)
                    {
                        System.out.println("secure datagrams are not supported");
                    }
                    else
                    {
                        datagram.sayHello(delay);
                    }
                }
                else if(line.equals("D"))
                {
                    if(secure)
                    {
                        System.out.println("secure datagrams are not supported");
                    }
                    else
                    {
                        batchDatagram.sayHello(delay);
                    }
                }
                else if(line.equals("f"))
                {
                    communicator().flushBatchRequests();
                }
                else if(line.equals("T"))
                {
                    if(timeout == -1)
                    {
                        timeout = 2000;
                    }
                    else
                    {
                        timeout = -1;
                    }

                    twoway = (HelloPrx)twoway.ice_timeout(timeout);
                    oneway = (HelloPrx)oneway.ice_timeout(timeout);
                    batchOneway = (HelloPrx)batchOneway.ice_timeout(timeout);

                    if(timeout == -1)
                    {
                        System.out.println("timeout is now switched off");
                    }
                    else
                    {
                        System.out.println("timeout is now set to 2000ms");
                    }
                }
                else if(line.equals("P"))
                {
                    if(delay == 0)
                    {
                        delay = 2500;
                    }
                    else
                    {
                        delay = 0;
                    }

                    if(delay == 0)
                    {
                        System.out.println("server delay is now deactivated");
                    }
                    else
                    {
                        System.out.println("server delay is now set to 2500ms");
                    }
                }
                else if(line.equals("S"))
                {
                    secure = !secure;

                    twoway = (HelloPrx)twoway.ice_secure(secure);
                    oneway = (HelloPrx)oneway.ice_secure(secure);
                    batchOneway = (HelloPrx)batchOneway.ice_secure(secure);
                    datagram = (HelloPrx)datagram.ice_secure(secure);
                    batchDatagram = (HelloPrx)batchDatagram.ice_secure(secure);

                    if(secure)
                    {
                        System.out.println("secure mode is now on");
                    }
                    else
                    {
                        System.out.println("secure mode is now off");
                    }
                }
                else if(line.equals("s"))
                {
                    twoway.shutdown();
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

