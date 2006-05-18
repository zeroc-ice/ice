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
        public SessionKeepAliveThread(IceGrid.SessionPrx session)
	{
	    _session = session;
	    _timeout = 5000;
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
                    catch(Ice.LocalException ex)
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
        IceGrid.SessionManagerPrx sessionManager = 
	    IceGrid.SessionManagerPrxHelper.checkedCast(communicator().stringToProxy("DemoIceGrid/SessionManager"));
	if(sessionManager == null)
	{
            Console.WriteLine(": cound not contact session manager");
	    return 1;
	}

	string id;
	do
	{
	    Console.Out.Write("user id: ");
	    Console.Out.Flush();

	    id = Console.In.ReadLine();
	    if(id == null)
	    {
	        return 1;
	    }
	    id = id.Trim();
	}
	while(id.Length == 0);

	IceGrid.SessionPrx session = sessionManager.createLocalSession(id);

	SessionKeepAliveThread keepAlive = new SessionKeepAliveThread(session);
	Thread keepAliveThread = new Thread(new ThreadStart(keepAlive.run));
	keepAliveThread.Start();

	HelloPrx hello = null;
	try
	{
	    hello = HelloPrxHelper.checkedCast(session.allocateObjectById(communicator().stringToIdentity("hello")));
	}
	catch(IceGrid.AllocationException ex)
	{
	    Console.WriteLine(": could not allocate object: " + ex.reason);
	    return 1;
	}
	catch(IceGrid.ObjectNotRegisteredException ex)
	{
	    Console.WriteLine(": object not registered with registry");
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
	    Console.WriteLine(": could not release object: " + ex.reason);
	    return 1;
	}
	catch(IceGrid.ObjectNotRegisteredException ex)
	{
	    Console.WriteLine(": object not registered with registry");
	    return 1;
	}

        return 0;
    }

    public static void Main(string[] args)
    {
	Client app = new Client();
	int status = app.main(args, "config.client");
	System.Environment.Exit(status);
    }
}
