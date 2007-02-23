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
            System.out.println("Hi");
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

    static private class SessionRefreshThread extends Thread
    {
        SessionRefreshThread(Ice.Logger logger, long timeout, SessionPrx session)
        {
            _logger = logger;
            _session = session;
            _timeout = timeout;
        }

        synchronized public void
        run()
        {
            while(!_terminated)
            {
                try
                {
                    wait(_timeout);
                }
                catch(InterruptedException e)
                {
                }
                if(!_terminated)
                {
                    try
                    {
                        _session.refresh();
                    }
                    catch(Ice.LocalException ex)
                    {
                        _logger.warning("SessionRefreshThread: " + ex);
                        _terminated = true;
                    }
                }
            }
        }

        synchronized private void
        terminate()
        {
            _terminated = true;
            notify();
        }

        final private Ice.Logger _logger;
        final private SessionPrx _session;
        final private long _timeout;
        private boolean _terminated = false;
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

    public int
    run(String[] args)
    {
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
            _refresh = new SessionRefreshThread(communicator().getLogger(), 5000, _session);
            _refresh.start();
        }
            
        java.util.ArrayList hellos = new java.util.ArrayList();

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
                        HelloPrx hello = (HelloPrx)hellos.get(index);
                        hello.sayHello();
                    }
                    else
                    {
                        System.out.println("index is too high. " + hellos.size() + " exist so far. " +
                                           "Use 'c' to create a new hello object.");
                    }
                }
                else if(line.equals("c"))
                {
                    hellos.add(_session.createHello());
                    System.out.println("created hello object " + (hellos.size()-1));
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
        // The refresher thread must be terminated before destroy is
        // called, otherwise it might get ObjectNotExistException.
        //
        if(_refresh != null)
        {
            _refresh.terminate();
            try
            {
                _refresh.join();
            }
            catch(InterruptedException e)
            {
            }
            _refresh = null;
        }
        
        if(destroy && _session != null)
        {
            _session.destroy();
            _session = null;
        }
    }

    public static void
    main(String[] args)
    {
        Client app = new Client();
        int status = app.main("Client", args, "config.client");
        System.exit(status);
    }

    private SessionRefreshThread _refresh = null;
    private SessionPrx _session = null;
}
