// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
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
        _stream = new BasicStream(ref.getInstance());

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
	    IceUtil.Debug.Assert(_stream != null);
	}
        _stream.reset();

        writeHeader(operation, mode, context);
    }

    // Returns true if ok, false if user exception.
    public boolean
    invoke()
        throws LocalExceptionWrapper
    {
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_state == StateUnsent);
	}

        _stream.endWriteEncaps();

	_state = StateInProgress;
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
		_connection.sendRequest(_stream, this);

                if(_exception != null)
                {
		    //
		    // TODO- what we want to do is fill in the
		    // exception's stack trace, but there doesn't seem
		    // to be a way to do this yet in CLDC.
		    //
		    //_exception.fillInStackTrace();
		    
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
                    // Throw the exception wrapped in a LocalExceptionWrapper, to
                    // indicate that the request cannot be resent without
                    // potentially violating the "at-most-once" principle.
                    //
                    throw new LocalExceptionWrapper(_exception, false);
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
		_connection.sendRequest(_stream, null);
                break;
            }

            case Reference.ModeBatchOneway:
            {
		//
		// For batch oneways the same rules as for
		// regular oneways (see comment above)
		// apply.
		//
                _connection.finishBatchRequest(_stream);
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
        throws LocalExceptionWrapper
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
            throw new LocalExceptionWrapper(ex, false);
	}

	throw ex;
    }

    public void
    finished(BasicStream is)
    {
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_reference.getMode() == Reference.ModeTwoway); // Can only be called for twoways.
	
	    IceUtil.Debug.Assert(_state <= StateInProgress);
	}
	
	//
	// Only swap the stream if the given stream is not this Outgoing object stream!
	//
	if(is != _stream)
	{
	    _stream.swap(is);
	}

	byte replyStatus = _stream.readByte();
	
	switch(replyStatus)
	{
	    case ReplyStatus.replyOK:
	    {
		//
		// Input and output parameters are always sent in an
		// encapsulation, which makes it possible to forward
		// oneway requests as blobs.
		//
		_stream.startReadEncaps();
		_state = StateOK; // The state must be set last, in case there is an exception.
		break;
	    }
	    
	    case ReplyStatus.replyUserException:
	    {
		//
		// Input and output parameters are always sent in an
		// encapsulation, which makes it possible to forward
		// oneway requests as blobs.
		//
		_stream.startReadEncaps();
		_state = StateUserException; // The state must be set last, in case there is an exception.
		break;
	    }
	    
	    case ReplyStatus.replyObjectNotExist:
	    case ReplyStatus.replyFacetNotExist:
	    case ReplyStatus.replyOperationNotExist:
	    {
		Ice.RequestFailedException ex = null;
		switch(replyStatus)
		{
		    case ReplyStatus.replyObjectNotExist:
		    {
			ex = new Ice.ObjectNotExistException();
			break;
		    }
		    
		    case ReplyStatus.replyFacetNotExist:
		    {
			ex = new Ice.FacetNotExistException();
			break;
		    }
		    
		    case ReplyStatus.replyOperationNotExist:
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
		ex.id.__read(_stream);

                //
                // For compatibility with the old FacetPath.
                //
                String[] facetPath = _stream.readStringSeq();
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

		ex.operation = _stream.readString();
		_exception = ex;

		_state = StateLocalException; // The state must be set last, in case there is an exception.
		break;
	    }
	    
	    case ReplyStatus.replyUnknownException:
	    case ReplyStatus.replyUnknownLocalException:
	    case ReplyStatus.replyUnknownUserException:
	    {
		Ice.UnknownException ex = null;
		switch(replyStatus)
		{
		    case ReplyStatus.replyUnknownException:
		    {
			ex = new Ice.UnknownException();
			break;
		    }
		    
		    case ReplyStatus.replyUnknownLocalException:
		    {
			ex = new Ice.UnknownLocalException();
			break;
		    }
		    
		    case ReplyStatus.replyUnknownUserException: 
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
		
		ex.unknown = _stream.readString();
		_exception = ex;

		_state = StateLocalException; // The state must be set last, in case there is an exception.
		break;
	    }
	    
	    default:
	    {
		_exception = new Ice.ProtocolException("unknown reply status");
		_state = StateLocalException;
		break;
	    }
	}
    }

    public void
    finished(Ice.LocalException ex)
    {
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_reference.getMode() == Reference.ModeTwoway); // Can only be called for twoways.
	    IceUtil.Debug.Assert(_state <= StateInProgress);
	}

	_state = StateLocalException;
	_exception = ex;
    }

    public BasicStream
    stream()
    {
        return _stream;
    }

    private void
    writeHeader(String operation, Ice.OperationMode mode, java.util.Hashtable context)
    {
        switch(_reference.getMode())
        {
            case Reference.ModeTwoway:
            case Reference.ModeOneway:
            {
                _stream.writeBlob(Protocol.requestHdr);
                break;
            }

            case Reference.ModeBatchOneway:
            {
                _connection.prepareBatchRequest(_stream);
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

        _reference.getIdentity().__write(_stream);

        //
        // For compatibility with the old FacetPath.
        //
	String facet = _reference.getFacet();
        if(facet == null || facet.length() == 0)
        {
            _stream.writeStringSeq(null);
        }
        else
        {
            String[] facetPath = { facet };
            _stream.writeStringSeq(facetPath);
        }

        _stream.writeString(operation);

        _stream.writeByte((byte)mode.value());

        if(context == null)
        {
            context = _reference.getContext();
        }

        final int sz = context.size();
        _stream.writeSize(sz);
        if(sz > 0)
        {
	    java.util.Enumeration e = context.keys();
            while(e.hasMoreElements())
            {
		String key = (String)e.nextElement();
		String value = (String)context.get(key);
                _stream.writeString(key);
                _stream.writeString(value);
            }
        }

        //
        // Input and output parameters are always sent in an
        // encapsulation, which makes it possible to forward requests as
        // blobs.
        //
        _stream.startWriteEncaps();
    }

    public int
    state()
    {
        return _state;
    }

    private Ice.Connection _connection;
    private Reference _reference;
    private Ice.LocalException _exception;

    public static final int StateUnsent = 0;
    public static final int StateInProgress = 1;
    public static final int StateOK = 2;
    public static final int StateUserException = 3;
    public static final int StateLocalException = 4;
    private int _state;

    private BasicStream _stream;

    public Outgoing next;
}
