// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public final class Outgoing
{
    public
    Outgoing(Connection connection, Reference ref, String operation, boolean nonmutating, java.util.Map context)
    {
        _connection = connection;
        _reference = ref;
        _state = StateUnsent;
        _is = new BasicStream(ref.instance);
        _os = new BasicStream(ref.instance);

        writeHeader(operation, nonmutating, context);
    }

    public void
    reset(String operation, boolean nonmutating, java.util.Map context)
    {
        _state = StateUnsent;
        _exception = null;
        _fillStackTrace = false;

        _is.reset();
        _os.reset();

        writeHeader(operation, nonmutating, context);
    }

    public void
    destroy()
    {
        if (_state == StateUnsent &&
            (_reference.mode == Reference.ModeBatchOneway ||
             _reference.mode == Reference.ModeBatchDatagram))
        {
            _connection.abortBatchRequest();
        }

        _os.destroy();
        _is.destroy();
    }

    // Returns true if ok, false if user exception.
    public boolean
    invoke()
        throws Ice.LocationForward, NonRepeatable
    {
        _os.endWriteEncaps();

        switch (_reference.mode)
        {
            case Reference.ModeTwoway:
            {
                boolean timedOut = false;

                synchronized(this)
                {
                    _connection.sendRequest(this, false);
                    _state = StateInProgress;

                    int timeout = _connection.timeout();
                    while (_state == StateInProgress)
                    {
                        try
                        {
                            if (timeout >= 0)
                            {
                                wait(timeout);
                                if (_state == StateInProgress)
                                {
                                    timedOut = true;
                                    _state = StateLocalException;
                                    _exception = new Ice.TimeoutException();
                                }
                            }
                            else
                            {
                                wait();
                            }
                        }
                        catch (InterruptedException ex)
                        {
                        }
                    }
                }

                if (timedOut)
                {
                    //
                    // Must be called outside the synchronization of
                    // this object
                    //
                    _connection.exception(_exception);
                }

                if (_exception != null)
                {
                    //      
                    // A CloseConnectionException indicates graceful
                    // server shutdown, and is therefore always repeatable
                    // without violating "at-most-once". That's because by
                    // sending a close connection message, the server
                    // guarantees that all outstanding requests can safely
                    // be repeated.
                    //
                    if (_exception instanceof Ice.CloseConnectionException)
                    {
                        throw _exception;
                    }

                    //
                    // If _fillStackTrace is true, then we want to update
                    // the exception's stack trace to reflect the calling
                    // thread.
                    //
                    if (_fillStackTrace)
                    {
                        _exception.fillInStackTrace();
                    }

                    //
                    // Throw the exception wrapped in a NonRepeatable, to
                    // indicate that the request cannot be resent without
                    // potentially violating the "at-most-once" principle.
                    //
                    throw new NonRepeatable(_exception);
                }

                if (_state == StateUserException)
                {
                    return false;
                }

                if (_state == StateLocationForward)
                {
                    Ice.ObjectPrx p = _is.readProxy();
                    throw new Ice.LocationForward(p);
                }

                assert(_state == StateOK);
                break;
            }

            case Reference.ModeOneway:
            case Reference.ModeDatagram:
            {
                _connection.sendRequest(this, true);
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
                _connection.finishBatchRequest(this);
                break;
            }
        }

        return true;
    }

    public synchronized void
    finished(BasicStream is)
    {
        if (_state == StateInProgress)
        {
            _is.swap(is);
            byte status = _is.readByte();
            switch ((int)status)
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

                case DispatchStatus._DispatchLocationForward:
                {
                    _state = StateLocationForward;
                    break;
                }

                case DispatchStatus._DispatchObjectNotExist:
                {
                    _state = StateLocalException;
                    Ice.ObjectNotExistException ex = new Ice.ObjectNotExistException();
		    ex.identity = new Ice.Identity();
		    ex.identity.__read(_is);
		    _exception = ex;
                    _fillStackTrace = true;
                    break;
                }

                case DispatchStatus._DispatchFacetNotExist:
                {
                    _state = StateLocalException;
                    Ice.FacetNotExistException ex = new Ice.FacetNotExistException();
		    ex.facet = _is.readString();
		    _exception = ex;
                    _fillStackTrace = true;
                    break;
                }

                case DispatchStatus._DispatchOperationNotExist:
                {
                    _state = StateLocalException;
                    Ice.OperationNotExistException ex = new Ice.OperationNotExistException();
		    ex.operation = _is.readString();
		    _exception = ex;
                    _fillStackTrace = true;
                    break;
                }

                case DispatchStatus._DispatchUnknownLocalException:
                {
                    _state = StateLocalException;
                    _exception = new Ice.UnknownLocalException();
                    _fillStackTrace = true;
                    break;
                }

                case DispatchStatus._DispatchUnknownUserException:
                {
                    _state = StateLocalException;
                    _exception = new Ice.UnknownUserException();
                    _fillStackTrace = true;
                    break;
                }

                case DispatchStatus._DispatchUnknownException:
                {
                    _state = StateLocalException;
                    _exception = new Ice.UnknownException();
                    _fillStackTrace = true;
                    break;
                }

                default:
                {
                    _state = StateLocalException;
                    _exception = new Ice.UnknownReplyStatusException();
                    _fillStackTrace = true;
                    break;
                }
            }
        }
        notify();
    }

    public synchronized void
    finished(Ice.LocalException ex)
    {
        if (_state == StateInProgress)
        {
            _state = StateLocalException;
            _exception = ex;
            notify();
        }
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
    writeHeader(String operation, boolean nonmutating, java.util.Map context)
    {
        switch (_reference.mode)
        {
            case Reference.ModeTwoway:
            case Reference.ModeOneway:
            case Reference.ModeDatagram:
            {
                _connection.prepareRequest(this);
                break;
            }

            case Reference.ModeBatchOneway:
            case Reference.ModeBatchDatagram:
            {
                _connection.prepareBatchRequest(this);
                break;
            }
        }

        _reference.identity.__write(_os);
        _os.writeString(_reference.facet);
        _os.writeString(operation);
        _os.writeBool(nonmutating);
        if (context == null)
        {
            _os.writeSize(0);
        }
        else
        {
            final int sz = context.size();
            _os.writeSize(sz);
            if (sz > 0)
            {
                java.util.Iterator i = context.entrySet().iterator();
                while (i.hasNext())
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
    private boolean _fillStackTrace;

    private static final int StateUnsent = 0;
    private static final int StateInProgress = 1;
    private static final int StateOK = 2;
    private static final int StateLocationForward = 3;
    private static final int StateUserException = 4;
    private static final int StateLocalException = 5;
    private int _state;

    private BasicStream _is;
    private BasicStream _os;

    public Outgoing next; // For use by Ice._ObjectDelM
}
