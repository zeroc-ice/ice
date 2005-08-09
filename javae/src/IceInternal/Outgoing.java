// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class Outgoing
{
    public
    Outgoing(Ice.Connection connection, Reference ref, String operation, Ice.OperationMode mode,
	     java.util.Hashtable context)
    {
        _connection = connection;
        _reference = ref;
        _state = StateUnsent;
        _is = new BasicStream(ref.getInstance());
        _os = new BasicStream(ref.getInstance());

        writeHeader(operation, mode, context);
    }

    //
    // This function allows this object to be reused, rather than reallocated.
    //
    public void
    reset(Reference ref, String operation, Ice.OperationMode mode, java.util.Hashtable context)
    {
	_reference = ref;
        _state = StateUnsent;
        _exception = null;

	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_is != null);
	}
        _is.reset();

	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_os != null);
	}
        _os.reset();

        writeHeader(operation, mode, context);
    }

    // Returns true if ok, false if user exception.
    public boolean
    invoke()
        throws NonRepeatable
    {
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_state == StateUnsent);
	}

        _os.endWriteEncaps();

        switch(_reference.getMode())
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
		_connection.sendRequest(_os, this);

		//
		// Wait until the request has completed, or until the
		// request times out.
		//

		boolean timedOut = false;

                synchronized(this)
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
                        try
                        {
                            if(timeout >= 0)
                            {
                                wait(timeout);
				
                                if(_state == StateInProgress)
                                {
                                    timedOut = true;
                                }
                            }
                            else
                            {
                                wait();
                            }
                        }
                        catch(InterruptedException ex)
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
                    _connection.exception(new Ice.TimeoutException());

		    //
		    // We must wait until the exception set above has
		    // propagated to this Outgoing object.
		    //
		    synchronized(this)
		    {
			while(_state == StateInProgress)
			{
			    try
			    {
				wait();
			    }
			    catch(InterruptedException ex)
			    {
			    }
			}
		    }
                }

                if(_exception != null)
                {
		    //
		    // TODO- what we want to do is fill in the exception's stack trace, but there doesn't seem to be
		    // a way to do this yet in CLDC.
		    //
		    
                    //      
                    // A CloseConnectionException indicates graceful
                    // server shutdown, and is therefore always repeatable
                    // without violating "at-most-once". That's because by
                    // sending a close connection message, the server
                    // guarantees that all outstanding requests can safely
                    // be repeated.
                    //
		    // An ObjectNotExistException can always be retried as
		    // well without violating "at-most-once".
		    //
                    if(_exception instanceof Ice.CloseConnectionException || 
		       _exception instanceof Ice.ObjectNotExistException)
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
		
		if(IceUtil.Debug.ASSERT)
		{
		    IceUtil.Debug.Assert(_state == StateOK);
		}
                break;
            }

            case Reference.ModeOneway:
            {
		//
		// For oneway requests, the connection object
                // never calls back on this object. Therefore we don't
                // need to lock the mutex or save exceptions. We simply
                // let all exceptions from sending propagate to the
                // caller, because such exceptions can be retried without
                // violating "at-most-once".
		//
		_state = StateInProgress;
		_connection.sendRequest(_os, null);
                break;
            }

            case Reference.ModeBatchOneway:
            {
		//
		// For batch oneways the same rules as for
		// regular oneways (see comment above)
		// apply.
		//
		_state = StateInProgress;
                _connection.finishBatchRequest(_os);
                break;
            }

	    case Reference.ModeDatagram:
	    case Reference.ModeBatchDatagram:
	    {
		if(IceUtil.Debug.ASSERT)
		{
	    	    IceUtil.Debug.Assert(false);
		}
	        return false;
	    }
        }

        return true;
    }

    public void
    abort(Ice.LocalException ex)
        throws NonRepeatable
    {
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_state == StateUnsent);
	}

	//
	// If we didn't finish a batch oneway request, we
	// must notify the connection about that we give up ownership
	// of the batch stream.
	//
	int mode = _reference.getMode();
	if(mode == Reference.ModeBatchOneway)
	{
	    _connection.abortBatchRequest();

	    //
	    // If we abort a batch requests, we cannot retry, because
	    // not only the batch request that caused the problem will
	    // be aborted, but all other requests in the batch as
	    // well.
	    //
	    throw new NonRepeatable(ex);
	}

	throw ex;
    }

    public synchronized void
    finished(BasicStream is)
    {
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_reference.getMode() == Reference.ModeTwoway); // Can only be called for twoways.
	
	    IceUtil.Debug.Assert(_state <= StateInProgress);
	}
	
	_is.swap(is);
	int status = (int)_is.readByte();
	
	switch(status)
	{
	    case DispatchStatus._DispatchOK:
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
	    
	    case DispatchStatus._DispatchUserException:
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
	    
	    case DispatchStatus._DispatchObjectNotExist:
	    case DispatchStatus._DispatchFacetNotExist:
	    case DispatchStatus._DispatchOperationNotExist:
	    {
		Ice.RequestFailedException ex = null;
		switch((int)status)
		{
		    case DispatchStatus._DispatchObjectNotExist:
		    {
			ex = new Ice.ObjectNotExistException();
			break;
		    }
		    
		    case DispatchStatus._DispatchFacetNotExist:
		    {
			ex = new Ice.FacetNotExistException();
			break;
		    }
		    
		    case DispatchStatus._DispatchOperationNotExist:
		    {
			ex = new Ice.OperationNotExistException();
			break;
		    }
		    
		    default:
		    {
			if(IceUtil.Debug.ASSERT)
			{
			    IceUtil.Debug.Assert(false);
			}
			break;
		    }
		}
		
		ex.id = new Ice.Identity();
		ex.id.__read(_is);

                //
                // For compatibility with the old FacetPath.
                //
                String[] facetPath = _is.readStringSeq();
                if(facetPath.length > 0)
                {
		    if(facetPath.length > 1)
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
	    
	    case DispatchStatus._DispatchUnknownException:
	    case DispatchStatus._DispatchUnknownLocalException:
	    case DispatchStatus._DispatchUnknownUserException:
	    {
		Ice.UnknownException ex = null;
		switch((int)status)
		{
		    case DispatchStatus._DispatchUnknownException:
		    {
			ex = new Ice.UnknownException();
			break;
		    }
		    
		    case DispatchStatus._DispatchUnknownLocalException:
		    {
			ex = new Ice.UnknownLocalException();
			break;
		    }
		    
		    case DispatchStatus._DispatchUnknownUserException: 
		    {
			ex = new Ice.UnknownUserException();
			break;
		    }
		    
		    default:
		    {
			if(IceUtil.Debug.ASSERT)
			{
			    IceUtil.Debug.Assert(false);
			}
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

        notify();
    }

    public synchronized void
    finished(Ice.LocalException ex)
    {
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_reference.getMode() == Reference.ModeTwoway); // Can only be called for twoways.
	
	    IceUtil.Debug.Assert(_state <= StateInProgress);
	}

	_state = StateLocalException;
	_exception = ex;
	notify();
    }

    public BasicStream
    is()
    {
        return _is;
    }

    public BasicStream
    os()
    {
        return _os;
    }

    private void
    writeHeader(String operation, Ice.OperationMode mode, java.util.Hashtable context)
    {
        switch(_reference.getMode())
        {
            case Reference.ModeTwoway:
            case Reference.ModeOneway:
            {
                _connection.prepareRequest(_os);
                break;
            }

            case Reference.ModeBatchOneway:
            {
                _connection.prepareBatchRequest(_os);
                break;
            }

	    case Reference.ModeDatagram:
	    case Reference.ModeBatchDatagram:
	    {
		if(IceUtil.Debug.ASSERT)
		{
	    	    IceUtil.Debug.Assert(false);
		}
	        break;
	    }
        }

        _reference.getIdentity().__write(_os);

        //
        // For compatibility with the old FacetPath.
        //
	String facet = _reference.getFacet();
        if(facet == null || facet.length() == 0)
        {
            _os.writeStringSeq(null);
        }
        else
        {
            String[] facetPath = { facet };
            _os.writeStringSeq(facetPath);
        }

        _os.writeString(operation);

        _os.writeByte((byte)mode.value());

        if(context == null)
        {
            _os.writeSize(0);
        }
        else
        {
            final int sz = context.size();
            _os.writeSize(sz);
            if(sz > 0)
            {
		java.util.Enumeration e = context.keys();
                while(e.hasMoreElements())
                {
		    String key = (String)e.nextElement();
		    String value = (String)context.get(key);
                    _os.writeString(key);
                    _os.writeString(value);
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

    private Ice.Connection _connection;
    private Reference _reference;
    private Ice.LocalException _exception;

    private static final int StateUnsent = 0;
    private static final int StateInProgress = 1;
    private static final int StateOK = 2;
    private static final int StateUserException = 3;
    private static final int StateLocalException = 4;
    private int _state;

    private BasicStream _is;
    private BasicStream _os;

    public Outgoing next; // For use by Ice._ObjectDelM
}
