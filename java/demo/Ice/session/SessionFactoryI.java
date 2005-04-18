// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class SessionFactoryI extends _SessionFactoryDisp
{
    private static class ReapThread extends Thread
    {
	ReapThread(SessionFactoryI factory, long timeout)
	{
	    _timeout = timeout;
	    _factory = factory;
	}

	public synchronized void
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
		    _factory.reap();
		}
	    }
	}

	synchronized void
	terminate()
	{
	    _terminated = true;
	    notify();
	}

	private boolean _terminated = false;
	final private long _timeout;
	final private SessionFactoryI _factory;
    };

    public
    SessionFactoryI(Ice.ObjectAdapter adapter)
    {
	_adapter = adapter;
	_reapThread = new ReapThread(this, _timeout);
	_reapThread.start();
    }

    public synchronized SessionPrx
    create(Ice.Current c)
    {
	SessionI session = new SessionI(_adapter, _timeout);
	SessionPrx proxy = SessionPrxHelper.uncheckedCast(_adapter.addWithUUID(session));
	_sessions.add(new SessionId(session, proxy.ice_getIdentity()));
	return proxy;
    }

    public void
    shutdown(Ice.Current c)
    {
	System.out.println("Shutting down...");
	c.adapter.getCommunicator().shutdown();
    }

    synchronized public void
    destroy()
    {
	_reapThread.terminate();
	try
	{
	    _reapThread.join();
	}
	catch(InterruptedException e)
	{
	}
	
	java.util.Iterator p = _sessions.iterator();
	while(p.hasNext())
	{
	    SessionId s = (SessionId)p.next();
	    s.session.destroyCallback();

	    // When the session factory is destroyed the OA is
	    // deactivated and all servants have been removed so
	    // calling remove on the OA is not necessary.
	}
	_sessions.clear();
    }

    synchronized private void
    reap()
    {
	java.util.Iterator p = _sessions.iterator();
	while(p.hasNext())
	{
	    SessionId s = (SessionId)p.next();
    	    if(s.session.destroyed())
	    {
		s.session.destroyCallback();
		try
		{
		    _adapter.remove(s.id);
		}
		catch(Ice.ObjectAdapterDeactivatedException e)
		{
		    // This method can be called while the server is
		    // shutting down, in which case this exception is
		    // expected.
		}
    	    	p.remove();
	    }
	}
    }
    

    final private long _timeout = 10 * 1000; // 10 seconds.
    final private Ice.ObjectAdapter _adapter;
    private ReapThread _reapThread;

    private static class SessionId
    {
    	SessionId(SessionI s, Ice.Identity i)
	{
	    session = s;
	    id = i;
	}
	final SessionI session;
    	final Ice.Identity id;
    };
    private java.util.List _sessions = new java.util.LinkedList();
}
