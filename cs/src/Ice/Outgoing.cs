// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Threading;

    public interface OutgoingMessageCallback
    {
        void sent(bool notify);
        void finished(Ice.LocalException ex, bool sent);
    }

    public class Outgoing : OutgoingMessageCallback
    {
        public Outgoing(RequestHandler handler, string operation, Ice.OperationMode mode,
                        Dictionary<string, string> context)
        {
            _state = StateUnsent;
            _sent = false;
            _handler = handler;

            Instance instance = _handler.getReference().getInstance();
            _is = new BasicStream(instance);
            _os = new BasicStream(instance);

            writeHeader(operation, mode, context);
        }

        //
        // These functions allow this object to be reused, rather than reallocated.
        //
        public void reset(RequestHandler handler, string operation, Ice.OperationMode mode,
                          Dictionary<string, string> context)
        {
            _state = StateUnsent;
            _exception = null;
            _sent = false;
            _handler = handler;

            writeHeader(operation, mode, context);
        }

        public void reclaim()
        {
            _is.reset();
            _os.reset();
        }

        // Returns true if ok, false if user exception.
        public bool invoke()
        {
            Debug.Assert(_state == StateUnsent);

            _os.endWriteEncaps();

            switch(_handler.getReference().getMode())
            {
                case Reference.Mode.ModeTwoway:
                {
                    _state = StateInProgress;

                    Ice.ConnectionI connection = _handler.sendRequest(this);

                    bool timedOut = false;

                    lock(this)
                    {

                        //
                        // If the request is being sent in the background we first wait for the
                        // sent notification.
                        //
                        while(_state != StateFailed && !_sent)
                        {
                            Monitor.Wait(this);
                        }

                        //
                        // Wait until the request has completed, or until the request
                        // times out.
                        //
                        int timeout = connection.timeout();
                        while(_state == StateInProgress && !timedOut)
                        {
                            if(timeout >= 0)
                            {
                                Monitor.Wait(this, timeout);

                                if(_state == StateInProgress)
                                {
                                    timedOut = true;
                                }
                            }
                            else
                            {
                                Monitor.Wait(this);
                            }
                        }
                    }

                    if(timedOut)
                    {
                        //
                        // Must be called outside the synchronization of
                        // this object
                        //
                        connection.exception(new Ice.TimeoutException());

                        //
                        // We must wait until the exception set above has
                        // propagated to this Outgoing object.
                        //
                        lock(this)
                        {
                            while(_state == StateInProgress)
                            {
                                Monitor.Wait(this);
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
                        // An ObjectNotExistException can always be retried as
                        // well without violating "at-most-once" (see the
                        // implementation of the checkRetryAfterException
                        // method of the ProxyFactory class for the reasons
                        // why it can be useful).
                        //
                        if(!_sent ||
                           _exception is Ice.CloseConnectionException ||
                           _exception is Ice.ObjectNotExistException)
                        {
                            throw _exception;
                        }

                        //
                        // Throw the exception wrapped in a LocalExceptionWrapper,
                        // to indicate that the request cannot be resent without
                        // potentially violating the "at-most-once" principle.
                        //
                        throw new LocalExceptionWrapper(_exception, false);
                    }

                    if(_state == StateUserException)
                    {
                        return false;
                    }
                    else
                    {
                        Debug.Assert(_state == StateOK);
                        return true;
                    }
                }

                case Reference.Mode.ModeOneway:
                case Reference.Mode.ModeDatagram:
                {
                    _state = StateInProgress;
                    if(_handler.sendRequest(this) != null)
                    {
                        //
                        // If the handler returns the connection, we must wait for the sent callback.
                        //
                        lock(this)
                        {
                            while(_state != StateFailed && !_sent)
                            {
                                Monitor.Wait(this);
                            }

                            if(_exception != null)
                            {
                                Debug.Assert(!_sent);
                                throw _exception;
                            }
                        }
                    }
                    return true;
                }

                case Reference.Mode.ModeBatchOneway:
                case Reference.Mode.ModeBatchDatagram:
                {
                    //
                    // For batch oneways and datagrams, the same rules
                    // as for regular oneways and datagrams (see
                    // comment above) apply.
                    //
                    _state = StateInProgress;
                    _handler.finishBatchRequest(_os);
                    return true;
                }
            }

            Debug.Assert(false);
            return false;
        }

        public void abort(Ice.LocalException ex)
        {
            Debug.Assert(_state == StateUnsent);

            //
            // If we didn't finish a batch oneway or datagram request,
            // we must notify the connection about that we give up
            // ownership of the batch stream.
            //
            Reference.Mode mode = _handler.getReference().getMode();
            if(mode == Reference.Mode.ModeBatchOneway || mode == Reference.Mode.ModeBatchDatagram)
            {
                _handler.abortBatchRequest();
            }

            throw ex;
        }

        public void sent(bool notify)
        {
            if(notify)
            {
                lock(this)
                {
                    _sent = true;
                    Monitor.Pulse(this);
                }
            }
            else
            {
                //
                // No synchronization is necessary if called from sendRequest() because the connection
                // send mutex is locked and no other threads can call on Outgoing until it's released.
                //
                _sent = true;
            }
        }

        public void finished(BasicStream istr)
        {
            lock(this)
            {
                Debug.Assert(_handler.getReference().getMode() == Reference.Mode.ModeTwoway); // Only for twoways.

                Debug.Assert(_state <= StateInProgress);

                _is.swap(istr);
                byte replyStatus = _is.readByte();

                switch(replyStatus)
                {
                    case ReplyStatus.replyOK:
                    {
                        _state = StateOK; // The state must be set last, in case there is an exception.
                        break;
                    }

                    case ReplyStatus.replyUserException:
                    {
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
                                Debug.Assert(false);
                                break;
                            }
                        }

                        ex.id = new Ice.Identity();
                        ex.id.read__(_is);

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

                Monitor.Pulse(this);
            }
        }

        public void finished(Ice.LocalException ex, bool sent)
        {
            lock(this)
            {
                Debug.Assert(_state <= StateInProgress);
                _state = StateFailed;
                _exception = ex;
                _sent = sent;
                Monitor.Pulse(this);
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

        public void throwUserException()
        {
            try
            {
                _is.startReadEncaps();
                _is.throwException();
            }
            catch(Ice.UserException)
            {
                _is.endReadEncaps();
                throw;
            }
        }

        private void writeHeader(string operation, Ice.OperationMode mode, Dictionary<string, string> context)
        {
            switch(_handler.getReference().getMode())
            {
                case Reference.Mode.ModeTwoway:
                case Reference.Mode.ModeOneway:
                case Reference.Mode.ModeDatagram:
                {
                    _os.writeBlob(IceInternal.Protocol.requestHdr);
                    break;
                }

                case Reference.Mode.ModeBatchOneway:
                case Reference.Mode.ModeBatchDatagram:
                {
                    _handler.prepareBatchRequest(_os);
                    break;
                }
            }

            try
            {
                _handler.getReference().getIdentity().write__(_os);

                //
                // For compatibility with the old FacetPath.
                //
                string facet = _handler.getReference().getFacet();
                if(facet == null || facet.Length == 0)
                {
                    _os.writeStringSeq(null);
                }
                else
                {
                    string[] facetPath = { facet };
                    _os.writeStringSeq(facetPath);
                }

                _os.writeString(operation);

                _os.writeByte((byte)mode);

                if(context != null)
                {
                    //
                    // Explicit context
                    //
                    Ice.ContextHelper.write(_os, context);
                }
                else
                {
                    //
                    // Implicit context
                    //
                    Ice.ImplicitContextI implicitContext = _handler.getReference().getInstance().getImplicitContext();
                    Dictionary<string, string> prxContext = _handler.getReference().getContext();

                    if(implicitContext == null)
                    {
                        Ice.ContextHelper.write(_os, prxContext);
                    }
                    else
                    {
                        implicitContext.write(prxContext, _os);
                    }
                }

                //
                // Input and output parameters are always sent in an
                // encapsulation, which makes it possible to forward requests as
                // blobs.
                //
                _os.startWriteEncaps();
            }
            catch(Ice.LocalException ex)
            {
                abort(ex);
            }
        }

        internal RequestHandler _handler;
        internal BasicStream _is;
        internal BasicStream _os;
        internal bool _sent;

        private Ice.LocalException _exception;

        private const int StateUnsent = 0;
        private const int StateInProgress = 1;
        private const int StateOK = 2;
        private const int StateUserException = 3;
        private const int StateLocalException = 4;
        private const int StateFailed = 5;
        private int _state;

        public Outgoing next; // For use by Ice.ObjectDelM_
    }

    public class BatchOutgoing : OutgoingMessageCallback
    {
        public BatchOutgoing(Ice.ConnectionI connection, Instance instance)
        {
            _connection = connection;
            _sent = false;
            _os = new BasicStream(instance);
        }

        public BatchOutgoing(RequestHandler handler)
        {
            _handler = handler;
            _sent = false;
            _os = new BasicStream(handler.getReference().getInstance());
        }

        public void invoke()
        {
            Debug.Assert(_handler != null || _connection != null);

            if(_handler != null && !_handler.flushBatchRequests(this) ||
               _connection != null && !_connection.flushBatchRequests(this))
            {
                lock(this)
                {
                    while(_exception == null && !_sent)
                    {
                        Monitor.Wait(this);
                    }

                    if(_exception != null)
                    {
                        throw _exception;
                    }
                }
            }
        }

        public void sent(bool notify)
        {
            if(notify)
            {
                lock(this)
                {
                    _sent = true;
                    Monitor.Pulse(this);
                }
            }
            else
            {
                _sent = true;
            }
        }

        public void finished(Ice.LocalException ex, bool sent)
        {
            lock(this)
            {
                _exception = ex;
                Monitor.Pulse(this);
            }
        }

        public BasicStream ostr()
        {
            return _os;
        }

        private RequestHandler _handler;
        private Ice.ConnectionI _connection;
        private BasicStream _os;
        private bool _sent;
        private Ice.LocalException _exception;
    }

}
