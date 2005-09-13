// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class CallbackSenderI extends _CallbackSenderDisp implements java.lang.Runnable
{
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
	System.out.println("adding client `" + Ice.Util.identityToString(ident) + "'");

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

		java.util.Iterator p = _clients.iterator();
		while(p.hasNext())
		{
		    CallbackReceiverPrx r = (CallbackReceiverPrx)p.next();
		    try
		    {
			r.callback(_num);
		    }
		    catch(Exception ex)
		    {
			System.out.println("removing client `" + Ice.Util.identityToString(r.ice_getIdentity()) + "':");
			ex.printStackTrace();
			p.remove();
		    }
		}
	    }
	}
    }

    private boolean _destroy = false;
    private int _num = 0;
    private java.util.Vector _clients = new java.util.Vector();
}
