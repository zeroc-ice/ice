// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using Demo;

public class QueueI : QueueDisp_
{
    public override void get_async(AMD_Queue_get cb, string id, Ice.Current current)
    {
        //
        // If there is already a message in the message queue, send the 
        // response  immediately. Otherwise add the callback to the 
        // request queue.
        //
        lock(this)
	{
            if(_messageQueue.Count != 0)
	    {
	        try
	        {
	            cb.ice_response((string)_messageQueue[0]);
		    _messageQueue.RemoveAt(0);
	        }
	        catch(Ice.Exception ex)
	        {
	            Console.Error.WriteLine(ex);
	        }
	    }
	    else
	    {
	        Request request = new Request();
		request.id = id;
		request.cb = cb;
	        _requestQueue.Add(request);
	    }
	}
    }

    public override void add(string message, Ice.Current current)
    {
        //
        // If there is an outstanding request in the request queue,
        // send a response. Otherwise add the message to the message
        // queue.
        //
        lock(this)
	{
            if(_requestQueue.Count != 0)
	    {
	        try
	        {
	            Request request = (Request)_requestQueue[0];
		    request.cb.ice_response(message);
	        }
	        catch(Ice.Exception ex)
	        {
	            Console.Error.WriteLine(ex);
	        }
		_requestQueue.RemoveAt(0);
	    }
	    else
	    {
	        _messageQueue.Add(message);
	    }
	}
    }

    public override void cancel_async(AMD_Queue_cancel cb, string[] ids, Ice.Current current)
    {
        //
        // We send immediate response so that later call to ice_exception
        // on queued requests will not cause deadlocks.
        //
        cb.ice_response();

	for(int i = 0; i < ids.Length; ++i)
	{
	    ArrayList toRemove = new ArrayList();
	    foreach(Request r in _requestQueue)
	    {
	        if(r.id.Equals(ids[i]))
		{
		    try
		    {
		        r.cb.ice_exception(new RequestCanceledException());
		    }
		    catch(Ice.Exception)
		    {
		        // Ignore
		    }
		    toRemove.Add(r);
		}
	    }

	    foreach(Request r in toRemove)
	    {
	        _requestQueue.Remove(r);
	    }
	}
    }


    class Request
    {
        public string id;
	public AMD_Queue_get cb;
    }

    private ArrayList _messageQueue = new ArrayList();
    private ArrayList _requestQueue = new ArrayList();
}
