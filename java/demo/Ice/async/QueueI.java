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
    get_async(AMD_Queue_get getCB, Ice.Current current)
    {
        if(_messageQueue.size() != 0)
	{
	    try
	    {
	        getCB.ice_response((String)_messageQueue.getFirst());
		_messageQueue.removeFirst();
	    }
	    catch(Ice.LocalException ex)
	    {
	        ex.printStackTrace();
	    }
	}
	else
	{
	    _requestQueue.add(getCB);
	}
    }

    synchronized public void
    add(String message, Ice.Current current)
    {
        if(_requestQueue.size() != 0)
	{
	    try
	    {
	        AMD_Queue_get cb = (AMD_Queue_get)_requestQueue.removeFirst();
		cb.ice_response(message);
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

    private java.util.LinkedList _messageQueue = new java.util.LinkedList();
    private java.util.LinkedList _requestQueue = new java.util.LinkedList();
}
