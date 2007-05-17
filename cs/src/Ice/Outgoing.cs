// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
        public Outgoing(Ice.ConnectionI connection, Reference r, string operation, Ice.OperationMode mode,
                        Ice.Context context, bool compress)
        {
            _connection = connection;
            _reference = r;
            _state = StateUnsent;
            _is = new BasicStream(r.getInstance());
            _os = new BasicStream(r.getInstance());
            _compress = compress;
            
            writeHeader(operation, mode, context);
        }

        //
        // These functions allow this object to be reused, rather than reallocated.
        //
        public void reset(Reference r, string operation, Ice.OperationMode mode, Ice.Context context, bool compress)
        {
            _reference = r;
            _state = StateUnsent;
            _exception = null;
            _compress = compress;
            
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
            
            switch(_reference.getMode())
            {
                case Reference.Mode.ModeTwoway: 
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
                        // An ObjectNotExistException can always be retried as
                        // well without violating "at-most-once".
                        //
                        if(_exception is Ice.CloseConnectionException || _exception is Ice.ObjectNotExistException)
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
                    
                    Debug.Assert(_state == StateOK);
                    break;
                }
                
                case Reference.Mode.ModeOneway: 
                case Reference.Mode.ModeDatagram: 
                {
                    //
                    // For oneway and datagram requests, the
                    // connection object never calls back on this
                    // object. Therefore we don't need to lock the
                    // mutex or save exceptions. We simply let all
                    // exceptions from sending propagate to the
                    // caller, because such exceptions can be retried
                    // without violating "at-most-once".
                    //
                    _state = StateInProgress;
                    _connection.sendRequest(_os, null, _compress);
                    break;
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
                    _connection.finishBatchRequest(_os, _compress);
                    break;
                }
            }
            
            return true;
        }
        
        public void abort(Ice.LocalException ex)
        {
            Debug.Assert(_state == StateUnsent);
            
            //
            // If we didn't finish a batch oneway or datagram request,
            // we must notify the connection about that we give up
            // ownership of the batch stream.
            //
            if(_reference.getMode() == Reference.Mode.ModeBatchOneway ||
               _reference.getMode() == Reference.Mode.ModeBatchDatagram)
            {
                _connection.abortBatchRequest();
 
                //
                // If we abort a batch requests, we cannot retry,
                // because not only the batch request that caused the
                // problem will be aborted, but all other requests in
                // the batch as well.
                //
                throw new LocalExceptionWrapper(ex, false);
            }
            
            throw ex;
        }

        public void finished(BasicStream istr)
        {
            lock(this)
            {
                Debug.Assert(_reference.getMode() == Reference.Mode.ModeTwoway); // Can only be called for twoways.

                Debug.Assert(_state <= StateInProgress);

                _is.swap(istr);
                byte replyStatus = _is.readByte();

                switch(replyStatus)
                {
                    case ReplyStatus.replyOK:
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
                
                    case ReplyStatus.replyUserException:
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

                System.Threading.Monitor.Pulse(this);
            }
        }
        
        public void finished(Ice.LocalException ex)
        {
            lock(this)
            {
                Debug.Assert(_reference.getMode() == Reference.Mode.ModeTwoway); // Can only be called for twoways.
            
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
            switch(_reference.getMode())
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
                    _connection.prepareBatchRequest(_os);
                    break;
                }
            }

            try
            {
                _reference.getIdentity().write__(_os);

                //
                // For compatibility with the old FacetPath.
                //
                string facet = _reference.getFacet();
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
                    Ice.ImplicitContextI implicitContext =
                        _reference.getInstance().getImplicitContext();
                    
                    Ice.Context prxContext = _reference.getContext();
                    
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
        
        private Ice.ConnectionI _connection;
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

        private bool _compress; // Immutable after construction
        
        public Outgoing next; // For use by Ice.ObjectDelM_
    }

}
