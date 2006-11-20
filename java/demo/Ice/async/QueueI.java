// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class QueueI extends _QueueDisp
{
    synchronized public void
    get_async(AMD_Queue_get cb, String id, Ice.Current current)
    {
        //
        // If there is already a message in the message queue, send the 
        // response  immediately. Otherwise add the callback to the 
        // request queue.
        //
        if(_messageQueue.size() != 0)
	{
	    try
	    {
	        cb.ice_response((String)_messageQueue.getFirst());
		_messageQueue.removeFirst();
	    }
	    catch(Ice.LocalException ex)
	    {
	        ex.printStackTrace();
	    }
	}
	else
	{
	    Request request = new Request();
	    request.id = id;
	    request.cb = cb;
	    _requestQueue.add(request);
	}
    }

    synchronized public void
    add(String message, Ice.Current current)
    {
        //
        // If there is an outstanding request in the request queue,
        // send a response. Otherwise add the message to the message
        // queue.
        //
        if(_requestQueue.size() != 0)
	{
	    try
	    {
	        Request request = (Request)_requestQueue.removeFirst();
		request.cb.ice_response(message);
	    }
	    catch(Ice.LocalException ex)
	    {
	        ex.printStackTrace();
	    }
	}
	else
	{
	    _messageQueue.add(message);
	}
    }

    synchronized public void
    cancel_async(AMD_Queue_cancel cb, String[] ids, Ice.Current current)
    {
        //
        // We send immediate response so that later call to ice_exception
        // on queued requests will not cause deadlocks.
        //
	cb.ice_response();

	for(int i = 0; i < ids.length; ++i)
	{
	    java.util.Iterator p = _requestQueue.iterator();
	    while(p.hasNext())
	    {
		Request request = (Request)p.next();
		if(request.id.equals(ids[i]))
		{
		    try
		    {
		        request.cb.ice_exception(new RequestCanceledException());
		    }
		    catch(Ice.LocalException ex)
		    {
		        // Ignore
		    }
		    p.remove();
		}
	    }
	}
    }

    class Request
    {
        String id;
	AMD_Queue_get cb;
    }

    private java.util.LinkedList _messageQueue = new java.util.LinkedList();
    private java.util.LinkedList _requestQueue = new java.util.LinkedList();
}
