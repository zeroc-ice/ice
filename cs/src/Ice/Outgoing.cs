// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


namespace IceInternal
{

    using System.Collections;
    using System.Diagnostics;

    public class Outgoing
    {
	public Outgoing(Connection connection, Reference r, string operation, Ice.OperationMode mode,
	                Ice.Context context, bool compress)
	{
	    _connection = connection;
	    _reference = r;
	    _state = StateUnsent;
	    _is = new BasicStream(r.instance);
	    _os = new BasicStream(r.instance);
	    _compress = compress;
	    
	    writeHeader(operation, mode, context);
	}
	
	//
	// Do NOT use a finalizer, this would cause a severe performance
	// penalty! We must make sure that destroy() is called instead,
	// to reclaim resources.
	//
	public void destroy()
	{
	    Debug.Assert(_is != null);
	    _is.destroy();
	    _is = null;
	    
	    Debug.Assert(_os != null);
	    _os.destroy();
	    _os = null;
	}
	
	//
	// This function allows this object to be reused, rather than
	// reallocated.
	//
	public void reset(string operation, Ice.OperationMode mode, Ice.Context context)
	{
	    _state = StateUnsent;
	    _exception = null;
	    
	    Debug.Assert(_is != null);
	    _is.reset();

	    Debug.Assert(_os != null);
	    _os.reset();
	    
	    writeHeader(operation, mode, context);
	}
	
	// Returns true if ok, false if user exception.
	public bool invoke()
	{
	    _os.endWriteEncaps();
	    
	    switch(_reference.mode)
	    {
		case Reference.ModeTwoway: 
		{
		    //
		    // We let all exceptions raised by sending directly
		    // propagate to the caller, because they can be
		    // retried without violating "at-most-once". In case
		    // of such exceptions, the connection object does not
		    // call back on this object, so we don't need to lock
		    // the mutex, keep track of state, or save exceptions.
		    //
		    _connection.sendRequest(_os, this, _compress);

		    //
		    // Wait until the request has completed, or until the
		    // request times out.
		    //

		    bool timedOut = false;
		    
		    lock(this)
		    {
			//
			// It's possible that the request has already
			// completed, due to a regular response, or because of
			// an exception. So we only change the state to "in
			// progress" if it is still "unsent".
			//
			if(_state == StateUnsent)
			{
			    _state = StateInProgress;
			}

			int timeout = _connection.timeout();
			while(_state == StateInProgress && !timedOut)
			{
			    if(timeout >= 0)
			    {
				System.Threading.Monitor.Wait(this, timeout);
				
				if(_state == StateInProgress)
				{
				    timedOut = true;
				}
			    }
			    else
			    {
				System.Threading.Monitor.Wait(this);
			    }
			}
		    }
		    
		    if(timedOut)
		    {
			//
			// Must be called outside the synchronization of
			// this object
			//
			_connection.exception(new Ice.TimeoutException());

			//
			// We must wait until the exception set above has
			// propagated to this Outgoing object.
			//
			lock(this)
			{
			    while(_state == StateInProgress)
			    {
				System.Threading.Monitor.Wait(this);
			    }
			}
		    }
		    
		    if(_exception != null)
		    {
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
		    //
		    // For oneway and datagram requests, the connection
		    // object never calls back on this object. Therefore
		    // we don't need to lock the mutex, keep track of
		    // state, or save exceptions. We simply let all
		    // exceptions from sending propagate to the caller,
		    // because such exceptions can be retried without
		    // violating "at-most-once".
		    //
		    _connection.sendRequest(_os, null, _compress);
		    break;
		}
		
		case Reference.ModeBatchOneway: 
		case Reference.ModeBatchDatagram: 
		{
		    //
		    // For batch oneways and datagrams, the same rules as for
		    // regular oneways and datagrams (see comment above)
		    // apply.
		    //
		    _connection.finishBatchRequest(_os, _compress);
		    break;
		}
	    }
	    
	    return true;
	}
	
	public void finished(BasicStream istr)
	{
	    lock(this)
	    {
		Debug.Assert(_reference.mode == Reference.ModeTwoway); // Can only be called for twoways.

		Debug.Assert(_state <= StateInProgress);

		_is.swap(istr);
		DispatchStatus status = (DispatchStatus)_is.readByte();

		switch(status)
		{
		    case DispatchStatus.DispatchOK:
		    {
			//
			// Input and output parameters are always sent in an
			// encapsulation, which makes it possible to forward
			// oneway requests as blobs.
			//
			_is.startReadEncaps();
			_state = StateOK; // The state must be set last, in case there is an exception.
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
			_state = StateUserException; // The state must be set last, in case there is an exception.
			break;
		    }
    		
		    case DispatchStatus.DispatchObjectNotExist:
		    case DispatchStatus.DispatchFacetNotExist:
		    case DispatchStatus.DispatchOperationNotExist:
		    {
			Ice.RequestFailedException ex = null;
			switch(status)
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

                        //
                        // For compatibility with the old FacetPath.
                        //
                        string[] facetPath = _is.readStringSeq();
                        if(facetPath.Length > 0)
                        {
                            if(facetPath.Length > 1)
                            {
                                throw new Ice.MarshalException();
                            }
                            ex.facet = facetPath[0];
                        }
                        else
                        {
                            ex.facet = "";
                        }

			ex.operation = _is.readString();
			_exception = ex;

                        _state = StateLocalException; // The state must be set last, in case there is an exception.
			break;
		    }
    		
		    case DispatchStatus.DispatchUnknownException:
		    case DispatchStatus.DispatchUnknownLocalException:
		    case DispatchStatus.DispatchUnknownUserException:
		    {
			Ice.UnknownException ex = null;
			switch(status)
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

                        _state = StateLocalException; // The state must be set last, in case there is an exception.
			break;
		    }
    		
		    default:
		    {
			_exception = new Ice.UnknownReplyStatusException();
                        _state = StateLocalException;
			break;
		    }
		}

		System.Threading.Monitor.Pulse(this);
	    }
	}
	
	public void finished(Ice.LocalException ex)
	{
	    lock(this)
	    {
		Debug.Assert(_reference.mode == Reference.ModeTwoway); // Can only be called for twoways.
	    
		Debug.Assert(_state <= StateInProgress);

		_state = StateLocalException;
		_exception = ex;
		System.Threading.Monitor.Pulse(this);
	    }
	}
	
	public BasicStream istr()
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

            //
            // For compatibility with the old FacetPath.
            //
            if(_reference.facet == null || _reference.facet.Length == 0)
            {
                _os.writeStringSeq(null);
            }
            else
            {
                string[] facetPath = { _reference.facet };
                _os.writeStringSeq(facetPath);
            }

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

	private volatile bool _compress; // Immutable after construction
	
	public Outgoing next; // For use by Ice._ObjectDelM
    }

}
