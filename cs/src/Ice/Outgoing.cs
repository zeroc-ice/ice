// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

namespace IceInternal
{

using System.Collections;
using System.Diagnostics;

public class Outgoing
{
    public
    Outgoing(Connection connection, Reference r, string operation, Ice.OperationMode mode, Ice.Context context)
    {
	_connection = connection;
	_reference = r;
	_state = StateUnsent;
	_is = new BasicStream(r.instance);
	_os = new BasicStream(r.instance);
	
	writeHeader(operation, mode, context);
    }
    
    public void
    reset(string operation, Ice.OperationMode mode, Ice.Context context)
    {
	_state = StateUnsent;
	_exception = null;
	
	_is.reset();
	_os.reset();
	
	writeHeader(operation, mode, context);
    }
    
    public void
    destroy()
    {
	if(_state == StateUnsent &&
	    (_reference.mode == Reference.ModeBatchOneway ||
	     _reference.mode == Reference.ModeBatchDatagram))
	{
	    _connection.abortBatchRequest();
	}
	
	_os.destroy();
	_is.destroy();
    }
    
    // Returns true if ok, false if user exception.
    public bool
    invoke()
    {
	_os.endWriteEncaps();
	
	switch(_reference.mode)
	{
	    case Reference.ModeTwoway: 
	    {
		bool timedOut = false;
		
		lock(this)
		{
		    _connection.sendRequest(this, false);
		    _state = StateInProgress;
		    
		    int timeout = _connection.timeout();
		    while(_state == StateInProgress)
		    {
			try
			{
			    if(timeout >= 0)
			    {
				System.Threading.Monitor.Wait(this, System.TimeSpan.FromMilliseconds(timeout));
				if(_state == StateInProgress)
				{
				    timedOut = true;
				    _state = StateLocalException;
				    _exception = new Ice.TimeoutException();
				}
			    }
			    else
			    {
				System.Threading.Monitor.Wait(this);
			    }
			}
			catch(System.Threading.ThreadInterruptedException)
			{
			}
		    }
		}
		
		if(timedOut)
		{
		    //
		    // Must be called outside the synchronization of
		    // this object
		    //
		    _connection.exception(_exception);
		}
		
		if(_exception != null)
		{
		    // _exception.fillInStackTrace(); // TODO: how to do this in C#?
		    
		    //      
		    // A CloseConnectionException indicates graceful
		    // server shutdown, and is therefore always repeatable
		    // without violating "at-most-once". That's because by
		    // sending a close connection message, the server
		    // guarantees that all outstanding requests can safely
		    // be repeated.
		    //
		    if(_exception is Ice.CloseConnectionException)
		    {
			throw _exception;
		    }
		    
		    //
		    // Throw the exception wrapped in a NonRepeatable, to
		    // indicate that the request cannot be resent without
		    // potentially violating the "at-most-once" principle.
		    //
		    throw new NonRepeatable(_exception);
		}
		
		if(_state == StateUserException)
		{
		    return false;
		}
		
		Debug.Assert(_state == StateOK);
		break;
	    }
	    
	    case Reference.ModeOneway: 
	    case Reference.ModeDatagram: 
	    {
		try
		{
		    _connection.sendRequest(this, true);
		}
		catch(Ice.DatagramLimitException ex)
		{
		    throw new NonRepeatable(ex);
		}
		_state = StateInProgress;
		break;
	    }
	    
	    case Reference.ModeBatchOneway: 
	    case Reference.ModeBatchDatagram: 
	    {
		//
		// The state must be set to StateInProgress before calling
		// finishBatchRequest, because otherwise if
		// finishBatchRequest raises an exception, the destructor
		// of this class will call abortBatchRequest, and calling
		// both finishBatchRequest and abortBatchRequest is
		// illegal.
		//
		_state = StateInProgress;
		_connection.finishBatchRequest(_os);
		break;
	    }
	}
	
	return true;
    }
    
    public void
    finished(BasicStream istr)
    {
	lock(this)
	{
	    //
	    // The state might be StateLocalException if there was a
	    // timeout in invoke().
	    //
	    if(_state == StateInProgress)
	    {
		_is.swap(istr);
		byte status = _is.readByte();
		
		switch((DispatchStatus)status)
		{
		    case DispatchStatus.DispatchOK: 
		    {
			//
			// Input and output parameters are always sent in an
			// encapsulation, which makes it possible to forward
			// oneway requests as blobs.
			//
			_is.startReadEncaps();
			_state = StateOK;
			break;
		    }
		    
		    case DispatchStatus.DispatchUserException: 
		    {
			//
			// Input and output parameters are always sent in an
			// encapsulation, which makes it possible to forward
			// oneway requests as blobs.
			//
			_is.startReadEncaps();
			_state = StateUserException;
			break;
		    }
		    
		    case DispatchStatus.DispatchObjectNotExist: 
		    case DispatchStatus.DispatchFacetNotExist: 
		    case DispatchStatus.DispatchOperationNotExist: 
		    {
			_state = StateLocalException;
			
			Ice.RequestFailedException ex = null;
			switch((DispatchStatus)status)
			{
			    case DispatchStatus.DispatchObjectNotExist: 
			    {
				ex = new Ice.ObjectNotExistException();
				break;
			    }
			    
			    case DispatchStatus.DispatchFacetNotExist: 
			    {
				ex = new Ice.FacetNotExistException();
				break;
			    }
			    
			    case DispatchStatus.DispatchOperationNotExist: 
			    {
				ex = new Ice.OperationNotExistException();
				break;
			    }
			    
			    default: 
			    {
				Debug.Assert(false);
				break;
			    }
			}
			
			ex.id = new Ice.Identity();
			ex.id.__read(_is);
			ex.facet = _is.readFacetPath();
			ex.operation = _is.readString();
			_exception = ex;
			break;
		    }
		    
		    case DispatchStatus.DispatchUnknownException: 
		    case DispatchStatus.DispatchUnknownLocalException: 
		    case DispatchStatus.DispatchUnknownUserException: 
		    {
			_state = StateLocalException;
			
			Ice.UnknownException ex = null;
			switch((DispatchStatus)status)
			{
			    case DispatchStatus.DispatchUnknownException: 
			    {
				ex = new Ice.UnknownException();
				break;
			    }
			    
			    case DispatchStatus.DispatchUnknownLocalException: 
			    {
				ex = new Ice.UnknownLocalException();
				break;
			    }
			    
			    case DispatchStatus.DispatchUnknownUserException: 
			    {
				ex = new Ice.UnknownUserException();
				break;
			    }
			    
			    default: 
			    {
				Debug.Assert(false);
				break;
			    }
			    
			}
			
			ex.unknown = _is.readString();
			_exception = ex;
			break;
		    }
		    
		    default: 
		    {
			_state = StateLocalException;
			_exception = new Ice.UnknownReplyStatusException();
			break;
		    }
		    
		}
	    }
	    
	    System.Threading.Monitor.Pulse(this);
	}
    }
    
    public void
    finished(Ice.LocalException ex)
    {
	lock(this)
	{
	    //
	    // The state might be StateLocalException if there was a
	    // timeout in invoke().
	    //
	    if(_state == StateInProgress)
	    {
		_state = StateLocalException;
		_exception = ex;
		System.Threading.Monitor.Pulse(this);
	    }
	}
    }
    
    public BasicStream
    istr()
    {
	return _is;
    }
    
    public BasicStream ostr()
    {
	return _os;
    }
    
    private void writeHeader(string operation, Ice.OperationMode mode, Ice.Context context)
    {
	switch(_reference.mode)
	{
	    case Reference.ModeTwoway: 
	    case Reference.ModeOneway: 
	    case Reference.ModeDatagram: 
	    {
		_connection.prepareRequest(_os);
		break;
	    }
	    
	    case Reference.ModeBatchOneway: 
	    case Reference.ModeBatchDatagram: 
	    {
		_connection.prepareBatchRequest(_os);
		break;
	    }
	}
	
	_reference.identity.__write(_os);
	_os.writeStringSeq(_reference.facet);
	_os.writeString(operation);
	_os.writeByte((byte)mode);
	if(context == null)
	{
	    _os.writeSize(0);
	}
	else
	{
	    int sz = context.Count;
	    _os.writeSize(sz);
	    if(sz > 0)
	    {
		foreach(DictionaryEntry e in context)
		{
		    _os.writeString((string)e.Key);
		    _os.writeString((string)e.Value);
		}
	    }
	}
	
	//
	// Input and output parameters are always sent in an
	// encapsulation, which makes it possible to forward requests as
	// blobs.
	//
	_os.startWriteEncaps();
    }
    
    private Connection _connection;
    private Reference _reference;
    private Ice.LocalException _exception;
    
    private const int StateUnsent = 0;
    private const int StateInProgress = 1;
    private const int StateOK = 2;
    private const int StateUserException = 3;
    private const int StateLocalException = 4;
    private int _state;
    
    private BasicStream _is;
    private BasicStream _os;
    
    public Outgoing next; // For use by Ice._ObjectDelM
}

}
