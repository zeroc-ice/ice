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
	    "c:     create new hello\n" +
	    "0-9:   greet hello object\n" +
	    "s:     shutdown server\n" +
	    "x:     exit\n" +
	    "t:     exit without destroying the session\n" +
	    "?:     help\n");
    }

    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        Ice.Properties properties = communicator.getProperties();
        final String proxyProperty = "SessionFactory.Proxy";
        String proxy = properties.getProperty(proxyProperty);
        if(proxy.length() == 0)
        {
            System.err.println("property `" + proxyProperty + "' not set");
            return 1;
        }

        Ice.ObjectPrx base = communicator.stringToProxy(proxy);
        SessionFactoryPrx factory = SessionFactoryPrxHelper.checkedCast(base);
        if(factory == null)
        {
            System.err.println("invalid proxy");
            return 1;
        }

    	SessionPrx session = factory.create();
    	SessionRefreshThread refresh = new SessionRefreshThread(communicator.getLogger(), 5000, session);
    	refresh.start();

    	java.util.ArrayList hellos = new java.util.ArrayList();

        menu();

        java.io.BufferedReader in = new java.io.BufferedReader(new java.io.InputStreamReader(System.in));

	try
	{
	    boolean destroy = true;
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
                    hellos.add(session.createHello());
    	    	    System.out.println("created hello object " + (hellos.size()-1));
                }
                else if(line.equals("s"))
                {
		    factory.shutdown();
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
    	    if(destroy)
	    {
		session.destroy();
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

    	refresh.terminate();
	try
	{
	    refresh.join();
	}
	catch(InterruptedException e)
	{
	}
    
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
        catch(Ice.LocalException ex)
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
