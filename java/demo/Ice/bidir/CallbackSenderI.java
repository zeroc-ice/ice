// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class CallbackSenderI extends _CallbackSenderDisp implements java.lang.Runnable
{
    CallbackSenderI()
    {
	_destroy = false;
	_num = 0;
	_lock = new java.lang.Object();
	_clients = new java.util.Vector();
    }

    public void
    destroy()
    {
	synchronized(_lock) // TODO: Make the method synchronized, remove _lock
	{
	    System.out.println("destroying callback sender");
	    _destroy = true;

	    _lock.notify();
	}
    }

    public void
    addClient(Ice.Identity ident, Ice.Current current)
    {
	synchronized(_lock)
	{
	    System.out.println("adding client `" + Ice.Util.identityToString(ident) + "'");

	    Ice.ObjectPrx base = current.con.createProxy(ident);
	    CallbackReceiverPrx client = CallbackReceiverPrxHelper.uncheckedCast(base);
	    _clients.addElement(client);
	}
    }

    public void
    run()
    {
	while(!_destroy) // TODO: Bug, check is not in synchronization.
	{
	    synchronized(_lock)
	    {
		try
		{
		    _lock.wait(2000);
		}
		catch(java.lang.InterruptedException ex)
		{
		}

		if(!_destroy && !_clients.isEmpty())
		{
		    ++_num;

		    java.util.Iterator p = _clients.iterator();
		    while(p.hasNext())
		    {
			try
			{
			    ((CallbackReceiverPrx)p.next()).callback(_num);
			}
			catch(Exception ex)
			{
			    p.remove(); // TODO: Check if it is legal to remove the current iterator.
			}
		    }
		}
	    }
	}
    }

    private boolean _destroy;
    private int _num;
    private java.lang.Object _lock; // TODO: remove.
    private java.util.Vector _clients;
}
