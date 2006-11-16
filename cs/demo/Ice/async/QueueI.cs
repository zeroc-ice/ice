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
    public override void get_async(AMD_Queue_get getCB, Ice.Current current)
    {
        lock(this)
	{
            if(_messageQueue.Count != 0)
	    {
	        try
	        {
	            getCB.ice_response((string)_messageQueue[0]);
		    _messageQueue.RemoveAt(0);
	        }
	        catch(Ice.Exception ex)
	        {
	            Console.Error.WriteLine(ex);
	        }
	    }
	    else
	    {
	        _requestQueue.Add(getCB);
	    }
	}
    }

    public override void add(string message, Ice.Current current)
    {
        lock(this)
	{
            if(_requestQueue.Count != 0)
	    {
	        try
	        {
	            AMD_Queue_get cb = (AMD_Queue_get)_requestQueue[0];
		    cb.ice_response(message);
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

    private ArrayList _messageQueue = new ArrayList();
    private ArrayList _requestQueue = new ArrayList();
}
