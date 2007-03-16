// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class SessionPingThread extends Thread
{
    SessionPingThread(ChatSessionPrx session, long timeout)
    {
        _session = session;
	_destroy = false;
	_timeout = timeout*1000;
    }

    synchronized public void
    destroy()
    {
	_destroy = true;
	this.notify();
    }

    synchronized public void
    run()
    {
	while(!_destroy)
	{
	    try
	    {
		this.wait(_timeout);
	    }
	    catch(java.lang.InterruptedException ex)
	    {
	    }

	    if(_destroy)
	    {
	        break;
	    }

	    try
	    {
	        _session.ice_ping();
	    }
	    catch(Ice.LocalException ex)
	    {
	        break;
	    }
	}
    }

    private ChatSessionPrx _session;
    private boolean _destroy;
    private long _timeout;
}
