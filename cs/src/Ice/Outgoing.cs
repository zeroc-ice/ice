// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
    using Ice.Instrumentation;

    public interface OutgoingMessageCallback
    {
        bool send(Ice.ConnectionI connection, bool compress, bool response);
        void invokeCollocated(CollocatedRequestHandler handler);
        void sent();
        void finished(Ice.Exception ex, bool sent);
    }

    public class Outgoing : OutgoingMessageCallback
    {
        public Outgoing(Ice.ObjectPrxHelperBase proxy, string operation, Ice.OperationMode mode, 
                        Dictionary<string, string> context, bool explicitCtx)
        {
            _state = StateUnsent;
            _sent = false;
            _proxy = proxy;
            _mode = mode;
            _handler = null;
            _observer = IceInternal.ObserverHelper.get(proxy, operation, context);
            _encoding = Protocol.getCompatibleEncoding(_proxy.reference__().getEncoding());
            _os = new BasicStream(_proxy.reference__().getInstance(), Ice.Util.currentProtocolEncoding);

            Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(_proxy.reference__().getProtocol()));

            writeHeader(operation, mode, context, explicitCtx);
        }

        //
        // These functions allow this object to be reused, rather than reallocated.
        //
        public void reset(Ice.ObjectPrxHelperBase proxy, string operation, Ice.OperationMode mode,
                          Dictionary<string, string> context, bool explicitCtx)
        {
            _state = StateUnsent;
            _exception = null;
            _sent = false;
            _proxy = proxy;
            _mode = mode;
            _handler = null;
            _observer = IceInternal.ObserverHelper.get(proxy, operation, context);
            _encoding = Protocol.getCompatibleEncoding(_proxy.reference__().getEncoding());

            Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(_proxy.reference__().getProtocol()));

            writeHeader(operation, mode, context, explicitCtx);
        }

        public void detach()
        {
            if(_observer != null)
            {
                _observer.detach();
                _observer = null;
            }
        }

        public void reclaim()
        {
            if(_is != null)
            {
                _is.reset();
            }
            _os.reset();
        }

        // Returns true if ok, false if user exception.
        public bool invoke()
        {
            Debug.Assert(_state == StateUnsent);

            Reference.Mode mode = _proxy.reference__().getMode();
            if(mode == Reference.Mode.ModeBatchOneway || mode == Reference.Mode.ModeBatchDatagram)
            {
                _state = StateInProgress;
                _handler.finishBatchRequest(_os);
                return true;
            }
            
            int cnt = 0;
            while(true)
            {
                try
                {
                    _state = StateInProgress;
                    _exception = null;
                    _sent = false;

                    _handler = _proxy.getRequestHandler__(false);

                    if(_handler.sendRequest(this)) // Request sent and no response expected, we're done.
                    {
                        return true;
                    }

                    bool timedOut = false;
                    _m.Lock();
                    try
                    {
                        //
                        // If the handler says it's not finished, we wait until we're done.
                        //

                        int invocationTimeout = _proxy.reference__().getInvocationTimeout();
                        if(invocationTimeout > 0)
                        {
                            long now = Time.currentMonotonicTimeMillis();
                            long deadline = now + invocationTimeout;
                            while((_state == StateInProgress || !_sent) && _state != StateFailed && !timedOut)
                            {
                                _m.TimedWait((int)(deadline - now));
                                
                                if((_state == StateInProgress || !_sent) && _state != StateFailed)
                                {
                                    now = Time.currentMonotonicTimeMillis();
                                    timedOut = now >= deadline;
                                }
                            }
                        }
                        else
                        {
                            while((_state == StateInProgress || !_sent) && _state != StateFailed)
                            {
                                _m.Wait();
                            }
                        }
                    }
                    finally
                    {
                        _m.Unlock();
                    }

                    if(timedOut)
                    {
                        _handler.requestTimedOut(this);

                        //
                        // Wait for the exception to propagate. It's possible the request handler ignores
                        // the timeout if there was a failure shortly before requestTimedOut got called. 
                        // In this case, the exception should be set on the Outgoing.
                        //
                        _m.Lock();
                        try
                        {
                            while(_exception == null)
                            {
                                _m.Wait();
                            }
                        }
                        finally
                        {
                            _m.Unlock();
                        }
                    }

                    if(_exception != null)
                    {
                        throw _exception;
                    }
                    else
                    {
                        Debug.Assert(_state != StateInProgress);
                        return _state == StateOK;
                    }
                }
                catch(RetryException)
                {
                    _proxy.setRequestHandler__(_handler, null); // Clear request handler and retry.
                }
                catch(Ice.Exception ex)
                {
                    try
                    {
                        int interval = _proxy.handleException__(ex, _handler, _mode, _sent, ref cnt);
                        if(_observer != null)
                        {
                            _observer.retried(); // Invocation is being retried.
                        }
                        if(interval > 0)
                        {
                            System.Threading.Thread.Sleep(interval);
                        }
                    }
                    catch(Ice.Exception exc)
                    {
                        if(_observer != null)
                        {
                            _observer.failed(exc.ice_name());
                        }
                        throw exc;
                    }
                }
            }
        }

        public void abort(Ice.LocalException ex)
        {
            Debug.Assert(_state == StateUnsent);

            //
            // If we didn't finish a batch oneway or datagram request,
            // we must notify the connection about that we give up
            // ownership of the batch stream.
            //
            Reference.Mode mode = _proxy.reference__().getMode();
            if(mode == Reference.Mode.ModeBatchOneway || mode == Reference.Mode.ModeBatchDatagram)
            {
                _handler.abortBatchRequest();
            }

            throw ex;
        }

        public bool send(Ice.ConnectionI connection, bool compress, bool response)
        {
            return connection.sendRequest(this, compress, response);
        }
        
        public void
        invokeCollocated(CollocatedRequestHandler handler)
        {
            handler.invokeRequest(this);
        }

        public void sent()
        {
            _m.Lock();
            try
            {
                if(_proxy.reference__().getMode() != Reference.Mode.ModeTwoway)
                {
                    if(_remoteObserver != null)
                    {
                        _remoteObserver.detach();
                        _remoteObserver = null;
                    }
                    _state = StateOK;
                }
                _sent = true;
                _m.Notify();
            }
            finally
            {
                _m.Unlock();
            }
        }

        public void finished(BasicStream istr)
        {
            _m.Lock();
            try
            {
                Debug.Assert(_proxy.reference__().getMode() == Reference.Mode.ModeTwoway); // Only for twoways.

                Debug.Assert(_state <= StateInProgress);

                if(_remoteObserver != null)
                {
                    _remoteObserver.reply(istr.size() - Protocol.headerSize - 4);
                    _remoteObserver.detach();
                    _remoteObserver = null;
                }

                if(_is == null)
                {
                    _is = new IceInternal.BasicStream(_proxy.reference__().getInstance(), 
                                                      Ice.Util.currentProtocolEncoding);
                }
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
                        if(_observer != null)
                        {
                            _observer.userException();
                        }
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

                _m.Notify();
            }
            finally
            {
                _m.Unlock();
            }
        }

        public void finished(Ice.Exception ex, bool sent)
        {
            _m.Lock();
            try
            {
                //Debug.Assert(_state <= StateInProgress);
                if(_state > StateInProgress)
                {
                    //
                    // Response was already received but message
                    // didn't get removed first from the connection
                    // send message queue so it's possible we can be
                    // notified of failures. In this case, ignore the
                    // failure and assume the outgoing has been sent.
                    //
                    Debug.Assert(_state != StateFailed);
                    _sent = true;
                    _m.Notify();
                }
                
                if(_remoteObserver != null)
                {
                    _remoteObserver.failed(ex.ice_name());
                    _remoteObserver.detach();
                    _remoteObserver = null;
                }
                _state = StateFailed;
                _exception = ex;
                _sent = sent;
                _m.Notify();
            }
            finally
            {
                _m.Unlock();
            }
        }

        public BasicStream ostr()
        {
            return _os;
        }

        public BasicStream startReadParams()
        {
            _is.startReadEncaps();
            return _is;
        }

        public void endReadParams()
        {
            _is.endReadEncaps();
        }

        public void readEmptyParams()
        {
            _is.skipEmptyEncaps();
        }

        public byte[] readParamEncaps()
        {
            return _is.readEncaps(out _encoding);
        }

        public BasicStream startWriteParams(Ice.FormatType format)
        {
            _os.startWriteEncaps(_encoding, format);
            return _os;
        }

        public void endWriteParams()
        {
            _os.endWriteEncaps();
        }

        public void writeEmptyParams()
        {
            _os.writeEmptyEncaps(_encoding);
        }

        public void writeParamEncaps(byte[] encaps)
        {
            if(encaps == null || encaps.Length == 0)
            {
                _os.writeEmptyEncaps(_encoding);
            }
            else
            {
                _os.writeEncaps(encaps);
            }
        }

        public bool hasResponse()
        {
            return _is != null && !_is.isEmpty();
        }

        public void throwUserException()
        {
            try
            {
                _is.startReadEncaps();
                _is.throwException(null);
            }
            catch(Ice.UserException)
            {
                _is.endReadEncaps();
                throw;
            }
        }

        public void attachRemoteObserver(Ice.ConnectionInfo info, Ice.Endpoint endpt, int requestId, int sz)
        {
            if(_observer != null)
            {
                _remoteObserver = _observer.getRemoteObserver(info, endpt, requestId, sz);
                if(_remoteObserver != null)
                {
                    _remoteObserver.attach();
                }
            }
        }

        public void attachCollocatedObserver(int requestId)
        {
            if(_observer != null)
            {
                _remoteObserver = _observer.getCollocatedObserver(requestId, _os.size() - Protocol.headerSize - 4);
                if(_remoteObserver != null)
                {
                    _remoteObserver.attach();
                }
            }
        }

        private void writeHeader(string operation, Ice.OperationMode mode, Dictionary<string, string> context, 
                                 bool explicitCtx)
        {
            if(explicitCtx && context == null)
            {
                context = _emptyContext;
            }

            switch(_proxy.reference__().getMode())
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
                    while(true)
                    {
                        try
                        {
                            _handler = _proxy.getRequestHandler__(true);
                            _handler.prepareBatchRequest(_os);
                            break;
                        }
                        catch(RetryException)
                        {
                            _proxy.setRequestHandler__(_handler, null); // Clear request handler and retry.
                        }
                        catch(Ice.LocalException ex)
                        {
                            if(_observer != null)
                            {
                                _observer.failed(ex.ice_name());
                            }
                            _proxy.setRequestHandler__(_handler, null); // Clear request handler
                            throw ex;
                        }
                    }
                    break;
                }
            }

            try
            {
                _proxy.reference__().getIdentity().write__(_os);

                //
                // For compatibility with the old FacetPath.
                //
                string facet = _proxy.reference__().getFacet();
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
                    Ice.ImplicitContextI implicitContext = _proxy.reference__().getInstance().getImplicitContext();
                    Dictionary<string, string> prxContext = _proxy.reference__().getContext();

                    if(implicitContext == null)
                    {
                        Ice.ContextHelper.write(_os, prxContext);
                    }
                    else
                    {
                        implicitContext.write(prxContext, _os);
                    }
                }
            }
            catch(Ice.LocalException ex)
            {
                abort(ex);
            }
        }

        private Ice.ObjectPrxHelperBase _proxy;
        private Ice.OperationMode _mode;
        private RequestHandler _handler;
        private BasicStream _is;
        private BasicStream _os;
        private bool _sent;
        private Ice.Exception _exception;
        private Ice.EncodingVersion _encoding;

        private const int StateUnsent = 0;
        private const int StateInProgress = 1;
        private const int StateOK = 2;
        private const int StateUserException = 3;
        private const int StateLocalException = 4;
        private const int StateFailed = 5;
        private int _state;

        private InvocationObserver _observer;
        private RemoteObserver _remoteObserver;

        private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();

        private static Dictionary<string, string> _emptyContext = new Dictionary<string, string>();

        public Outgoing next; // For use by Ice.ObjectPrxHelperBase
    }

    public class BatchOutgoing : OutgoingMessageCallback
    {
        public BatchOutgoing(Ice.ConnectionI connection, Instance instance, string operation)
        {
            _connection = connection;
            _sent = false;
            _observer = IceInternal.ObserverHelper.get(instance, operation);
            _os = new BasicStream(instance, Ice.Util.currentProtocolEncoding);
        }

        public BatchOutgoing(Ice.ObjectPrxHelperBase proxy, string operation)
        {
            _proxy = proxy;
            _sent = false;
            _observer = IceInternal.ObserverHelper.get(proxy, operation);
            _os = new BasicStream(_proxy.reference__().getInstance(), Ice.Util.currentProtocolEncoding);
            Protocol.checkSupportedProtocol(_proxy.reference__().getProtocol());
        }

        public void invoke()
        {
            Debug.Assert(_proxy != null || _connection != null);

            if(_connection != null)
            {
                if(_connection.flushBatchRequests(this))
                {
                    return;
                }
                _m.Lock();
                try
                {
                    while(_exception == null && !_sent)
                    {
                        _m.Wait();
                    }
                    if(_exception != null)
                    {
                        throw _exception;
                    }
                }
                finally
                {
                    _m.Unlock();
                }
                return;
            }

            RequestHandler handler = null;
            try
            {
                handler = _proxy.getRequestHandler__(false);
                if(handler.sendRequest(this))
                {
                    return;
                }

                bool timedOut = false;
                _m.Lock();
                try
                {
                    int timeout = _proxy.reference__().getInvocationTimeout();
                    if(timeout > 0)
                    {
                        long now = Time.currentMonotonicTimeMillis();
                        long deadline = now + timeout;
                        while(_exception == null && !_sent && !timedOut)
                        {
                            _m.TimedWait((int)(deadline - now));
                            if(_exception == null && !_sent)
                            {
                                now = Time.currentMonotonicTimeMillis();
                                timedOut = now >= deadline;
                            }
                        }
                    }
                    else
                    {
                        while(_exception == null && !_sent)
                        {
                            _m.Wait();
                        }
                    }
                }
                finally
                {
                    _m.Unlock();
                }
                
                if(timedOut)
                {
                    handler.requestTimedOut(this);

                    _m.Lock();
                    try
                    {
                        while(_exception == null)
                        {
                            _m.Wait();
                        }
                    }
                    finally
                    {
                        _m.Unlock();
                    }
                }
                
                if(_exception != null)
                {
                    throw _exception;
                }
            }
            catch(RetryException)
            {
                //
                // Clear request handler but don't retry or throw. Retrying
                // isn't useful, there were no batch requests associated with
                // the proxy's request handler.
                //
                _proxy.setRequestHandler__(handler, null); 
            }
            catch(Ice.Exception ex)
            {
                _proxy.setRequestHandler__(handler, null); // Clear request handler
                if(_observer != null)
                {
                    _observer.failed(ex.ice_name());
                }
                throw ex; // Throw to notify the user that batch requests were potentially lost.
            }
        }


        public bool send(Ice.ConnectionI connection, bool compress, bool response)
        {
            return connection.flushBatchRequests(this);
        }
        
        public void invokeCollocated(CollocatedRequestHandler handler)
        {
            handler.invokeBatchRequests(this);
        }

        public void sent()
        {
            _m.Lock();
            try
            {
                if(_remoteObserver != null)
                {
                    _remoteObserver.detach();
                    _remoteObserver = null;
                }
                _sent = true;
                _m.Notify();
            }
            finally
            {
                _m.Unlock();
            }
        }

        public void finished(Ice.Exception ex, bool sent)
        {
            _m.Lock();
            if(_remoteObserver != null)
            {
                _remoteObserver.failed(ex.ice_name());
                _remoteObserver.detach();
            }
            try
            {
                _exception = ex;
                _m.Notify();
            }
            finally
            {
                _m.Unlock();
            }
        }

        public BasicStream ostr()
        {
            return _os;
        }

        public void attachRemoteObserver(Ice.ConnectionInfo info, Ice.Endpoint endpt, int size)
        {
            if(_observer != null)
            {
                _remoteObserver = _observer.getRemoteObserver(info, endpt, 0, size);
                if(_remoteObserver != null)
                {
                    _remoteObserver.attach();
                }
            }
        }

        public void attachCollocatedObserver(int requestId)
        {
            if(_observer != null)
            {
                _remoteObserver = _observer.getCollocatedObserver(requestId, _os.size() - Protocol.headerSize - 4);
                if(_remoteObserver != null)
                {
                    _remoteObserver.attach();
                }
            }
        }

        private Ice.ObjectPrxHelperBase _proxy;
        private Ice.ConnectionI _connection;
        private BasicStream _os;
        private bool _sent;
        private Ice.Exception _exception;

        private InvocationObserver _observer;
        private Observer _remoteObserver;

        private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
    }
}
