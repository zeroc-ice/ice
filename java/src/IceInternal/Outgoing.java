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
    Outgoing(Emitter emitter, Reference ref, boolean sendProxy,
             String operation, boolean nonmutating, java.util.HashMap context)
    {
        _emitter = emitter;
        _reference = ref;
        _state = StateUnsent;
        _is = new BasicStream(ref.instance);
        _os = new BasicStream(ref.instance);

        switch (_reference.mode)
        {
            case Reference.ModeTwoway:
            case Reference.ModeOneway:
            case Reference.ModeDatagram:
            {
                _emitter.prepareRequest(this);
                break;
            }

            case Reference.ModeBatchOneway:
            case Reference.ModeBatchDatagram:
            {
                _emitter.prepareBatchRequest(this);
                break;
            }
        }

        _os.writeBool(sendProxy);
        if (sendProxy)
        {
            Ice.ObjectPrx proxy = _reference.instance.proxyFactory().
                referenceToProxy(_reference);
            _os.writeProxy(proxy);
        }
        else
        {
            _reference.identity.__write(_os);
            _os.writeString(_reference.facet);
        }
        _os.writeString(operation);
        _os.writeBool(nonmutating);
        if (context == null)
        {
            _os.writeInt(0);
        }
        else
        {
            final int sz = context.size();
            _os.writeInt(sz);
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
        // encapsulation, which makes it possible to forward oneway
        // requests as blobs.
        //
        _os.startWriteEncaps();
    }

    protected void
    finalize()
        throws Throwable
    {
        if (_state == StateUnsent &&
            (_reference.mode == Reference.ModeBatchOneway ||
             _reference.mode == Reference.ModeBatchDatagram))
        {
            _emitter.abortBatchRequest();
        }

        super.finalize();
    }

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
                    _emitter.sendRequest(this, false);
                    _state = StateInProgress;

                    int timeout = _emitter.timeout();
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
                    _emitter.exception(_exception);
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
                    // Throw the exception wrapped in a NonRepeatable, to
                    // indicate that the request cannot be resent without
                    // potentially violating the "at-most-once" principle.
                    //
                    throw new NonRepeatable(_exception);
                }

                if (_state == StateException)
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
                _emitter.sendRequest(this, true);
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
                _emitter.finishBatchRequest(this);
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
                    _state = StateException;
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
                    _exception = new Ice.ObjectNotExistException();
                    break;
                }

                case DispatchStatus._DispatchFacetNotExist:
                {
                    _state = StateLocalException;
                    _exception = new Ice.FacetNotExistException();
                    break;
                }

                case DispatchStatus._DispatchOperationNotExist:
                {
                    _state = StateLocalException;
                    _exception = new Ice.OperationNotExistException();
                    break;
                }

                case DispatchStatus._DispatchUnknownLocalException:
                {
                    _state = StateLocalException;
                    _exception = new Ice.UnknownLocalException();
                    break;
                }

                case DispatchStatus._DispatchUnknownUserException:
                {
                    _state = StateLocalException;
                    _exception = new Ice.UnknownUserException();
                    break;
                }

                case DispatchStatus._DispatchUnknownException:
                {
                    _state = StateLocalException;
                    _exception = new Ice.UnknownException();
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

    private Emitter _emitter;
    private Reference _reference;
    private Ice.LocalException _exception;

    private static final int StateUnsent = 0;
    private static final int StateInProgress = 1;
    private static final int StateOK = 2;
    private static final int StateException = 3;
    private static final int StateLocationForward = 4;
    private static final int StateLocalException = 5;
    private int _state;

    private BasicStream _is;
    private BasicStream _os;
}
