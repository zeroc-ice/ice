// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class CallbackSenderI extends _CallbackSenderDisp implements java.lang.Runnable
{
    CallbackSenderI(Ice.Communicator communicator)
    {
        _communicator = communicator;
	_destroy = false;
	_num = 0;
	_clients = new java.util.Vector();
    }

    synchronized public void
    destroy()
    {
	System.out.println("destroying callback sender");
	_destroy = true;

	this.notify();
    }

    synchronized public void
    addClient(Ice.Identity ident, Ice.Current current)
    {
	System.out.println("adding client `" + _communicator.identityToString(ident) + "'");

	Ice.ObjectPrx base = current.con.createProxy(ident);
	CallbackReceiverPrx client = CallbackReceiverPrxHelper.uncheckedCast(base);
	_clients.addElement(client);
    }

    synchronized public void
    run()
    {
	while(!_destroy)
	{
	    try
	    {
		this.wait(2000);
	    }
	    catch(java.lang.InterruptedException ex)
	    {
	    }

	    if(!_destroy && !_clients.isEmpty())
	    {
		++_num;

		int i = 0;
		while(i < _clients.size())
		{
		    CallbackReceiverPrx r = (CallbackReceiverPrx)_clients.elementAt(i);
		    try
		    {
			r.callback(_num);
		    }
		    catch(Exception ex)
		    {
			System.out.println("removing client `" + 
					   _communicator.identityToString(r.ice_getIdentity()) + "':");
			ex.printStackTrace();
			_clients.removeElementAt(i);
			continue;
		    }
		    ++i;
		}
	    }
	}
    }

    private Ice.Communicator _communicator;
    private boolean _destroy;
    private int _num;
    private java.util.Vector _clients;
}
