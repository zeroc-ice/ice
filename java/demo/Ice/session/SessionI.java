// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class SessionI extends _SessionDisp
{
    public
    SessionI(Ice.ObjectAdapter adapter, long timeout)
    {
	_adapter = adapter;
	_timeout = timeout;
	_refreshTime = System.currentTimeMillis();
    }

    synchronized public HelloPrx
    createHello(Ice.Current c)
    {
	HelloPrx hello = HelloPrxHelper.uncheckedCast(_adapter.addWithUUID(new HelloI(_nextId++)));
	_objs.add(hello);
	return hello;
    }

    synchronized public void
    refresh(Ice.Current c)
    {
	_refreshTime = System.currentTimeMillis();
    }
    
    synchronized public void
    destroy(Ice.Current c)
    {
	_destroy = true;
    }
    
    // Return true if the session is destroyed, false otherwise.
    synchronized protected boolean
    destroyed()
    {
	return _destroy || (System.currentTimeMillis() - _refreshTime) > _timeout;
    }

    // Called when the session is destroyed. This should release any
    // per-client allocated resources.
    synchronized protected void
    destroyCallback()
    {
	System.out.println("SessionI.destroyCallback: _destroy=" + _destroy + " timeout=" +
			   ((System.currentTimeMillis()-_refreshTime) > _timeout));
	java.util.Iterator p = _objs.iterator();
	while(p.hasNext())
	{
	    HelloPrx h = (HelloPrx)p.next();
	    try
	    {
		_adapter.remove(h.ice_getIdentity());
	    }
	    catch(Ice.ObjectAdapterDeactivatedException e)
	    {
		// This method is called on shutdown of the server, in
		// which case this exception is expected.
	    }
	}
	_objs.clear();
    }

    final Ice.ObjectAdapter _adapter;
    final long _timeout; // How long until the session times out.

    private boolean _destroy = false; // true if destroy() was called, false otherwise.
    private long _refreshTime; // The last time the session was refreshed.
    private int _nextId = 0; // The id of the next hello object. This is used for tracing purposes.
    private java.util.List _objs = new java.util.LinkedList(); // List of per-client allocated Hello objects.
}
