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
    Outgoing(Emitter emitter, Reference ref)
    {
        _emitter = emitter;
        _reference = ref;
        _state = StateUnsent;
        _is = new StreamI(ref.instance);
        _os = new StreamI(ref.instance);

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

        _os.writeString(_reference.identity);
        _os.writeString(_reference.facet);
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
    {
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
                                    _state = StateLocalException;
                                    _exception = new Ice.TimeoutException();
                                    timedOut = true;
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

                if (_exception != null)
                {
                    if (timedOut)
                    {
                        //
                        // Must be called outside the synchronization of
                        // this object
                        //
                        _emitter.exception(_exception);
                    }

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
                _state = StateInProgress; // Must be set to StateInProgress
                                          // before finishBatchRequest()
                _emitter.finishBatchRequest(this);
                break;
            }
        }

        return true;
    }

    public synchronized void
    finished(Ice.Stream is)
    {
        assert(_state != StateUnsent);
        if (_state == StateInProgress)
        {
            _is.swap(is);
            byte status = _is.readByte();
            switch ((int)status)
            {
                case DispatchStatus._DispatchOK:
                {
                    _state = StateOK;
                    break;
                }

                case DispatchStatus._DispatchUserException:
                {
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
        assert(_state != StateUnsent);
        if (_state == StateInProgress)
        {
            _state = StateLocalException;
            _exception = ex;
            notify();
        }
    }

    public Ice.Stream
    is()
    {
        return _is;
    }

    public Ice.Stream
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

    private Ice.Stream _is;
    private Ice.Stream _os;
}
