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
            cleanup(true);
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
            "c:     create a new per-client hello object\n" +
            "0-9:   send a greeting to a hello object\n" +
            "s:     shutdown the server and exit\n" +
            "x:     exit\n" +
            "t:     exit without destroying the session\n" +
            "?:     help\n");
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

        java.io.BufferedReader in = new java.io.BufferedReader(new java.io.InputStreamReader(System.in));
        String name;
        try
        {
            do
            {
                System.out.print("Please enter your name ==> ");
                System.out.flush();
                name = in.readLine().trim();
            }
            while(name.length() == 0);
        }
        catch(java.io.IOException ex)
        {
            ex.printStackTrace();
            return 0;
        }

        Ice.ObjectPrx base = communicator().propertyToProxy("SessionFactory.Proxy");
        SessionFactoryPrx factory = SessionFactoryPrxHelper.checkedCast(base);
        if(factory == null)
        {
            System.err.println("invalid proxy");
            return 1;
        }

        synchronized(this)
        {
            _session = factory.create(name);
            _executor.scheduleAtFixedRate(new Runnable()
            {
                @Override
                public void
                run()
                {
                    try
                    {
                        _session.refresh();
                    }
                    catch(Ice.LocalException ex)
                    {
                        communicator().getLogger().warning("SessionRefreshThread: " + ex);
                        // Exceptions thrown from the executor task supress subsequent execution
                        // of the task.
                        throw ex;
                    }
                }
            }, 5, 5, java.util.concurrent.TimeUnit.SECONDS);
        }
            
        java.util.ArrayList<HelloPrx> hellos = new java.util.ArrayList<HelloPrx>();

        menu();

        try
        {
            boolean destroy = true;
            boolean shutdown = false;
            while(true)
            {
                System.out.print("==> ");
                System.out.flush();
                String line = in.readLine();
                if(line == null)
                {
                    break;
                }
                if(line.length() > 0 && Character.isDigit(line.charAt(0)))
                {
                    int index;
                    try
                    {
                        index = Integer.parseInt(line);
                    }
                    catch(NumberFormatException e)
                    {
                        menu();
                        continue;
                    }
                    if(index < hellos.size())
                    {
                        HelloPrx hello = hellos.get(index);
                        hello.sayHello();
                    }
                    else
                    {
                        System.out.println("Index is too high. " + hellos.size() + " exist so far. " +
                                           "Use 'c' to create a new hello object.");
                    }
                }
                else if(line.equals("c"))
                {
                    hellos.add(_session.createHello());
                    System.out.println("created hello object " + (hellos.size() - 1));
                }
                else if(line.equals("s"))
                {
                    destroy = false;
                    shutdown = true;
                    break;
                }
                else if(line.equals("x"))
                {
                    break;
                }
                else if(line.equals("t"))
                {
                    destroy = false;
                    break;
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

            cleanup(destroy);
            if(shutdown)
            {
                factory.shutdown();
            }
        }
        catch(Exception ex)
        {
            try
            {
                cleanup(true);
            }
            catch(Ice.LocalException e)
            {
            }
            ex.printStackTrace();
        }

        return 0;
    }

    synchronized private void
    cleanup(boolean destroy)
    {
        //
        // The refresher task must be terminated before destroy is
        // called, otherwise it might get ObjectNotExistException.
        //
        _executor.shutdown();       
        if(destroy && _session != null)
        {
            _session.destroy();
        }
        _session = null;
    }

    public static void
    main(String[] args)
    {
        Client app = new Client();
        int status = app.main("Client", args, "config.client");
        System.exit(status);
    }

    private java.util.concurrent.ScheduledExecutorService _executor = java.util.concurrent.Executors.newScheduledThreadPool(1);
    private SessionPrx _session = null;
}
