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

package IceInternal;

public final class Outgoing
{
    public
    Outgoing(Connection connection, Reference ref, String operation, Ice.OperationMode mode, java.util.Map context)
    {
        _connection = connection;
        _reference = ref;
        _state = StateUnsent;
        _is = new BasicStream(ref.instance);
        _os = new BasicStream(ref.instance);

        writeHeader(operation, mode, context);
    }

    public void
    reset(String operation, Ice.OperationMode mode, java.util.Map context)
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
        _os.destroy();
        _is.destroy();
    }

    // Returns true if ok, false if user exception.
    public boolean
    invoke()
        throws NonRepeatable
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
			
			assert(_exception != null);
		    }
                }

                if(_exception != null)
                {
		    _exception.fillInStackTrace();

                    //      
                    // A CloseConnectionException indicates graceful
                    // server shutdown, and is therefore always repeatable
                    // without violating "at-most-once". That's because by
                    // sending a close connection message, the server
                    // guarantees that all outstanding requests can safely
                    // be repeated.
                    //
                    if(_exception instanceof Ice.CloseConnectionException)
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
		
                assert(_state == StateOK);
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
		_connection.sendRequest(_os, null);
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
                _connection.finishBatchRequest(_os);
                break;
            }
        }

        return true;
    }

    public synchronized void
    finished(BasicStream is)
    {
	assert(_reference.mode == Reference.ModeTwoway); // Can only be called for twoways.
	
	assert(_state <= StateInProgress);
	
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
		_state = StateOK;
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
		_state = StateUserException;
		break;
	    }
	    
	    case DispatchStatus._DispatchObjectNotExist:
	    case DispatchStatus._DispatchFacetNotExist:
	    case DispatchStatus._DispatchOperationNotExist:
	    {
		_state = StateLocalException;
		
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
			assert(false);
			break;
		    }
		}
		
		ex.id = new Ice.Identity();
		ex.id.__read(_is);
		ex.facet = _is.readStringSeq();
		ex.operation = _is.readString();
		_exception = ex;
		break;
	    }
	    
	    case DispatchStatus._DispatchUnknownException:
	    case DispatchStatus._DispatchUnknownLocalException:
	    case DispatchStatus._DispatchUnknownUserException:
	    {
		_state = StateLocalException;
		
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
			assert(false);
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

        notify();
    }

    public synchronized void
    finished(Ice.LocalException ex)
    {
	assert(_reference.mode == Reference.ModeTwoway); // Can only be called for twoways.
	
	assert(_state <= StateInProgress);

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
    writeHeader(String operation, Ice.OperationMode mode, java.util.Map context)
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
                java.util.Iterator i = context.entrySet().iterator();
                while(i.hasNext())
                {
                    java.util.Map.Entry entry = (java.util.Map.Entry)i.next();
                    _os.writeString((String)entry.getKey());
                    _os.writeString((String)entry.getValue());
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
