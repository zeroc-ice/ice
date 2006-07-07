// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Threading;
using Demo;


public class Client : Ice.Application
{
    class SessionKeepAliveThread 
    {
        public SessionKeepAliveThread(IceGrid.SessionPrx session, int timeout)
	{
	    _session = session;
	    _timeout = timeout;
	    _terminated = false;
	}

	public void run()
	{
	    lock(this)
	    {
                while(!_terminated)
                {
	            System.Threading.Monitor.Wait(this, _timeout);
                    if(_terminated)
                    {
		        break;
		    }
                    try
                    {
                        _session.keepAlive();
                    }
                    catch(Ice.LocalException)
                    {
		        break;
                    }
                }
	    }
	}

	public void terminate()
	{
	    lock(this)
	    {
	        _terminated = true;
	        System.Threading.Monitor.Pulse(this);
	    }
	}

	private IceGrid.SessionPrx _session;
	private int _timeout;
	private bool _terminated;
    }

    private void menu()
    {
        Console.Out.WriteLine(
            "usage:\n" +
            "t: send greeting\n" +
            "x: exit\n" +
            "?: help\n");
    }

    public override int run(string[] args)
    {
        IceGrid.RegistryPrx registry = 
	    IceGrid.RegistryPrxHelper.checkedCast(communicator().stringToProxy("DemoIceGrid/Registry"));
	if(registry == null)
	{
            Console.WriteLine("could not contact registry");
	    return 1;
	}

	
	IceGrid.SessionPrx session = null;
	while(true)
	{
	    Console.Out.WriteLine("This demo accepts any user-id / password combination.");

	    string id;
	    Console.Out.Write("user id: ");
	    Console.Out.Flush();
	    id = Console.In.ReadLine();

	    string pw;
	    Console.Out.Write("password: ");
	    Console.Out.Flush();
	    pw = Console.In.ReadLine();

	    try
	    {
	        session = registry.createSession(id, pw);
		break;
	    }
	    catch(IceGrid.PermissionDeniedException ex)
	    {
	        Console.WriteLine("permission denied:\n" + ex.reason);
	    }
	}

	SessionKeepAliveThread keepAlive = new SessionKeepAliveThread(session, (int)registry.getSessionTimeout() / 2);
	Thread keepAliveThread = new Thread(new ThreadStart(keepAlive.run));
	keepAliveThread.Start();

	HelloPrx hello = null;
	try
	{
	    hello = HelloPrxHelper.checkedCast(session.allocateObjectById(communicator().stringToIdentity("hello")));
	}
	catch(IceGrid.AllocationException ex)
	{
	    Console.WriteLine("could not allocate object: " + ex.reason);
	    return 1;
	}
	catch(IceGrid.ObjectNotRegisteredException)
	{
	    Console.WriteLine("object not registered with registry");
	    return 1;
	}

        menu();

        string line = null;
        do
        {
            try
            {
                Console.Write("==> ");
                Console.Out.Flush();
                line = Console.In.ReadLine();
                if(line == null)
                {
                    break;
                }
                if(line.Equals("t"))
                {
                    hello.sayHello();
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
                    Console.WriteLine("unknown command `" + line + "'");
                    menu();
                }
            }
            catch(Ice.LocalException ex)
            {
                Console.WriteLine(ex);
            }
        }
        while(!line.Equals("x"));

	keepAlive.terminate();
	keepAliveThread.Join();
	keepAlive = null;
	
	try
	{
	    session.releaseObject(hello.ice_getIdentity());
	}
	catch(IceGrid.AllocationException ex)
	{
	    Console.WriteLine("could not release object: " + ex.reason);
	    return 1;
	}
	catch(IceGrid.ObjectNotRegisteredException)
	{
	    Console.WriteLine("object not registered with registry");
	    return 1;
	}

	session.destroy();

        return 0;
    }

    public static void Main(string[] args)
    {
	Client app = new Client();
	int status = app.main(args, "config.client");
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}
