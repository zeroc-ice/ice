// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Threading;

    public class OutgoingAsyncBase : AsyncResultI
    {
        public virtual Ice.AsyncCallback sent()
        {
            return sent(true);
        }

        public virtual Ice.AsyncCallback completed(Ice.Exception ex)
        {
            return finished(ex);
        }

        public virtual Ice.AsyncCallback completed()
        {
            Debug.Assert(false); // Must be implemented by classes that handle responses
            return null;
        }

        public void attachRemoteObserver(Ice.ConnectionInfo info, Ice.Endpoint endpt, int requestId)
        {
            if(observer_ != null)
            {
                int size = os_.size() - Protocol.headerSize - 4;
                childObserver_ = getObserver().getRemoteObserver(info, endpt, requestId, size);
                if(childObserver_ != null)
                {
                    childObserver_.attach();
                }
            }
        }

        public void attachCollocatedObserver(Ice.ObjectAdapter adapter, int requestId)
        {
            if(observer_ != null)
            {
                int size = os_.size() - Protocol.headerSize - 4;
                childObserver_ = getObserver().getCollocatedObserver(adapter, requestId, size);
                if(childObserver_ != null)
                {
                    childObserver_.attach();
                }
            }
        }

        public Ice.OutputStream getOs()
        {
            return os_;
        }

        public virtual Ice.InputStream getIs()
        {
            return null; // Must be implemented by classes that handle responses
        }

        protected OutgoingAsyncBase(Ice.Communicator com, Instance instance, string op, object cookie) :
            base(com, instance, op, cookie)
        {
            os_ = new Ice.OutputStream(instance, Ice.Util.currentProtocolEncoding);
        }

        protected OutgoingAsyncBase(Ice.Communicator com, Instance instance, string op, object cookie,
                                    Ice.OutputStream os) :
            base(com, instance, op, cookie)
        {
            os_ = os;
        }

        protected new Ice.AsyncCallback sent(bool done)
        {
            if(done)
            {
                if(childObserver_ != null)
                {
                    childObserver_.detach();
                    childObserver_ = null;
                }
            }
            return base.sent(done);
        }

        protected new Ice.AsyncCallback finished(Ice.Exception ex)
        {
            if(childObserver_ != null)
            {
                childObserver_.failed(ex.ice_id());
                childObserver_.detach();
                childObserver_ = null;
            }
            return base.finished(ex);
        }

        protected Ice.OutputStream os_;
        protected Ice.Instrumentation.ChildInvocationObserver childObserver_;
    }

    //
    // Base class for proxy based invocations. This class handles the
    // retry for proxy invocations. It also ensures the child observer is
    // correct notified of failures and make sure the retry task is
    // correctly canceled when the invocation completes.
    //
    public abstract class ProxyOutgoingAsyncBase : OutgoingAsyncBase, TimerTask
    {
        public static ProxyOutgoingAsyncBase check(Ice.AsyncResult r, Ice.ObjectPrx prx, string operation)
        {
            return ProxyOutgoingAsyncBase.check<ProxyOutgoingAsyncBase>(r, prx, operation);
        }

        public abstract bool invokeRemote(Ice.ConnectionI con, bool compress, bool resp, out Ice.AsyncCallback cb);

        public abstract bool invokeCollocated(CollocatedRequestHandler handler, out Ice.AsyncCallback cb);

        public override Ice.ObjectPrx getProxy()
        {
            return proxy_;
        }

        public override Ice.AsyncCallback completed(Ice.Exception exc)
        {
            if(childObserver_ != null)
            {
                childObserver_.failed(exc.ice_id());
                childObserver_.detach();
                childObserver_ = null;
            }

            cachedConnection_ = null;
            if(proxy_.reference__().getInvocationTimeout() == -2)
            {
                instance_.timer().cancel(this);
            }

            //
            // NOTE: at this point, synchronization isn't needed, no other threads should be
            // calling on the callback.
            //
            try
            {
                //
                // It's important to let the retry queue do the retry even if
                // the retry interval is 0. This method can be called with the
                // connection locked so we can't just retry here.
                //
                instance_.retryQueue().add(this, handleException(exc));
                return null;
            }
            catch(Ice.Exception ex)
            {
                return finished(ex); // No retries, we're done
            }
        }

        public void retryException(Ice.Exception ex)
        {
            try
            {
                //
                // It's important to let the retry queue do the retry. This is
                // called from the connect request handler and the retry might
                // require could end up waiting for the flush of the
                // connection to be done.
                //
                proxy_.updateRequestHandler__(handler_, null); // Clear request handler and always retry.
                instance_.retryQueue().add(this, 0);
            }
            catch(Ice.Exception exc)
            {
                Ice.AsyncCallback cb = completed(exc);
                if(cb != null)
                {
                    invokeCompletedAsync(cb);
                }
            }
        }

        public override void cancelable(CancellationHandler handler)
        {
            if(proxy_.reference__().getInvocationTimeout() == -2 && cachedConnection_ != null)
            {
                int timeout = cachedConnection_.timeout();
                if(timeout > 0)
                {
                    instance_.timer().schedule(this, timeout);
                }
            }
            base.cancelable(handler);
        }

        public void retry()
        {
            invokeImpl(false);
        }

        public virtual void abort(Ice.Exception ex)
        {
            Debug.Assert(childObserver_ == null);
            Ice.AsyncCallback cb = finished(ex);
            if(cb != null)
            {
                invokeCompletedAsync(cb);
            }
            else if(ex is Ice.CommunicatorDestroyedException)
            {
                //
                // If it's a communicator destroyed exception, don't swallow
                // it but instead notify the user thread. Even if no callback
                // was provided.
                //
                throw ex;
            }
        }

        public void runTimerTask()
        {
            if(proxy_.reference__().getInvocationTimeout() == -2)
            {
                cancel(new Ice.ConnectionTimeoutException());
            }
            else
            {
                cancel(new Ice.InvocationTimeoutException());
            }
        }

        protected ProxyOutgoingAsyncBase(Ice.ObjectPrxHelperBase prx, string op, object cookie) :
            base(prx.ice_getCommunicator(), prx.reference__().getInstance(), op, cookie)
        {
            proxy_ = prx;
            mode_ = Ice.OperationMode.Normal;
            _cnt = 0;
            _sent = false;
        }

        protected ProxyOutgoingAsyncBase(Ice.ObjectPrxHelperBase prx, string op, object cookie, Ice.OutputStream os) :
            base(prx.ice_getCommunicator(), prx.reference__().getInstance(), op, cookie, os)
        {
            proxy_ = prx;
            mode_ = Ice.OperationMode.Normal;
            _cnt = 0;
            _sent = false;
        }

        protected static T check<T>(Ice.AsyncResult r, Ice.ObjectPrx prx, string operation)
        {
            if(r != null && r.getProxy() != prx)
            {
                throw new System.ArgumentException("Proxy for call to end_" + operation +
                                                   " does not match proxy that was used to call corresponding begin_" +
                                                   operation + " method");
            }
            return check<T>(r, operation);
        }

        protected void invokeImpl(bool userThread)
        {
            try
            {
                if(userThread)
                {
                    int invocationTimeout = proxy_.reference__().getInvocationTimeout();
                    if(invocationTimeout > 0)
                    {
                        instance_.timer().schedule(this, invocationTimeout);
                    }
                }
                else // If not called from the user thread, it's called from the retry queue
                {
                    if(observer_ != null)
                    {
                        observer_.retried();
                    }
                }

                while(true)
                {
                    try
                    {
                        _sent = false;
                        handler_ = proxy_.getRequestHandler__();
                        Ice.AsyncCallback sentCallback;
                        if(handler_.sendAsyncRequest(this, out sentCallback))
                        {
                            if(userThread)
                            {
                                sentSynchronously_ = true;
                                if(sentCallback != null)
                                {
                                    invokeSent(sentCallback); // Call from the user thread.
                                }
                            }
                            else
                            {
                                if(sentCallback != null)
                                {
                                    invokeSentAsync(sentCallback); // Call from a client thread pool thread.
                                }
                            }
                        }
                        return; // We're done!
                    }
                    catch(RetryException)
                    {
                        proxy_.updateRequestHandler__(handler_, null); // Clear request handler and always retry.
                    }
                    catch(Ice.Exception ex)
                    {
                        if(childObserver_ != null)
                        {
                            childObserver_.failed(ex.ice_id());
                            childObserver_.detach();
                            childObserver_ = null;
                        }
                        int interval = handleException(ex);
                        if(interval > 0)
                        {
                            instance_.retryQueue().add(this, interval);
                            return;
                        }
                        else if(observer_ != null)
                        {
                            observer_.retried();
                        }
                    }
                }
            }
            catch(Ice.Exception ex)
            {
                //
                // If called from the user thread we re-throw, the exception
                // will be catch by the caller and abort() will be called.
                //
                if(userThread)
                {
                    throw;
                }
                Ice.AsyncCallback cb = finished(ex); // No retries, we're done
                if(cb != null)
                {
                    invokeCompletedAsync(cb);
                }
            }
        }

        protected new Ice.AsyncCallback sent(bool done)
        {
            _sent = true;
            if(done)
            {
                if(proxy_.reference__().getInvocationTimeout() != -1)
                {
                    instance_.timer().cancel(this);
                }
            }
            return base.sent(done);
        }

        protected new Ice.AsyncCallback finished(Ice.Exception ex)
        {
            if(proxy_.reference__().getInvocationTimeout() != -1)
            {
                instance_.timer().cancel(this);
            }
            return base.finished(ex);
        }

        protected new Ice.AsyncCallback finished(bool ok)
        {
            if(proxy_.reference__().getInvocationTimeout() != -1)
            {
                instance_.timer().cancel(this);
            }
            return base.finished(ok);
        }

        protected virtual int handleException(Ice.Exception exc)
        {
            return proxy_.handleException__(exc, handler_, mode_, _sent, ref _cnt);
        }

        protected Ice.ObjectPrxHelperBase proxy_;
        protected RequestHandler handler_;
        protected Ice.OperationMode mode_;

        private int _cnt;
        private bool _sent;
    }

    public class OutgoingAsync : ProxyOutgoingAsyncBase
    {
        public new static OutgoingAsync check(Ice.AsyncResult r, Ice.ObjectPrx prx, string operation)
        {
            return ProxyOutgoingAsyncBase.check<OutgoingAsync>(r, prx, operation);
        }

        public OutgoingAsync(Ice.ObjectPrx prx, string operation, object cookie) :
            base((Ice.ObjectPrxHelperBase)prx, operation, cookie)
        {
            _encoding = Protocol.getCompatibleEncoding(proxy_.reference__().getEncoding());
            _is = null;
        }

        public OutgoingAsync(Ice.ObjectPrx prx, string operation, object cookie, Ice.InputStream istr,
                             Ice.OutputStream ostr) :
            base((Ice.ObjectPrxHelperBase)prx, operation, cookie, ostr)
        {
            _encoding = Protocol.getCompatibleEncoding(proxy_.reference__().getEncoding());
            _is = istr;
        }

        public void prepare(string operation, Ice.OperationMode mode, Dictionary<string, string> ctx,
                            bool explicitCtx, bool synchronous)
        {
            Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(proxy_.reference__().getProtocol()));

            mode_ = mode;
            _synchronous = synchronous;

            if(explicitCtx && ctx == null)
            {
                ctx = _emptyContext;
            }
            observer_ = ObserverHelper.get(proxy_, operation, ctx);

            switch(proxy_.reference__().getMode())
            {
                case Reference.Mode.ModeTwoway:
                case Reference.Mode.ModeOneway:
                case Reference.Mode.ModeDatagram:
                {
                    os_.writeBlob(Protocol.requestHdr);
                    break;
                }

                case Reference.Mode.ModeBatchOneway:
                case Reference.Mode.ModeBatchDatagram:
                {
                    proxy_.getBatchRequestQueue__().prepareBatchRequest(os_);
                    break;
                }
            }

            Reference rf = proxy_.reference__();

            rf.getIdentity().write__(os_);

            //
            // For compatibility with the old FacetPath.
            //
            string facet = rf.getFacet();
            if(facet == null || facet.Length == 0)
            {
                os_.writeStringSeq(null);
            }
            else
            {
                string[] facetPath = { facet };
                os_.writeStringSeq(facetPath);
            }

            os_.writeString(operation);

            os_.writeByte((byte)mode);

            if(ctx != null)
            {
                //
                // Explicit context
                //
                Ice.ContextHelper.write(os_, ctx);
            }
            else
            {
                //
                // Implicit context
                //
                Ice.ImplicitContextI implicitContext = rf.getInstance().getImplicitContext();
                Dictionary<string, string> prxContext = rf.getContext();

                if(implicitContext == null)
                {
                    Ice.ContextHelper.write(os_, prxContext);
                }
                else
                {
                    implicitContext.write(prxContext, os_);
                }
            }
        }

        public override Ice.AsyncCallback sent()
        {
            return sent(!proxy_.ice_isTwoway()); // done = true if not a two-way proxy (no response expected)
        }

        public override bool invokeRemote(Ice.ConnectionI con, bool compress, bool resp, out Ice.AsyncCallback sentCB)
        {
            cachedConnection_ = con;
            return con.sendAsyncRequest(this, compress, resp, 0, out sentCB);
        }

        public override bool invokeCollocated(CollocatedRequestHandler handler, out Ice.AsyncCallback sentCB)
        {
            // The stream cannot be cached if the proxy is not a twoway or there is an invocation timeout set.
            if(!proxy_.ice_isTwoway() || proxy_.reference__().getInvocationTimeout() != -1)
            {
                // Disable caching by marking the streams as cached!
                state_ |= StateCachedBuffers;
            }
            return handler.invokeAsyncRequest(this, 0, _synchronous, out sentCB);
        }

        public override void abort(Ice.Exception ex)
        {
            Reference.Mode mode = proxy_.reference__().getMode();
            if(mode == Reference.Mode.ModeBatchOneway || mode == Reference.Mode.ModeBatchDatagram)
            {
                proxy_.getBatchRequestQueue__().abortBatchRequest(os_);
            }

            base.abort(ex);
        }

        public void invoke()
        {
            Reference.Mode mode = proxy_.reference__().getMode();
            if(mode == Reference.Mode.ModeBatchOneway || mode == Reference.Mode.ModeBatchDatagram)
            {
                sentSynchronously_ = true;
                proxy_.getBatchRequestQueue__().finishBatchRequest(os_, proxy_, getOperation());
                finished(true);
                return; // Don't call sent/completed callback for batch AMI requests
            }

            //
            // NOTE: invokeImpl doesn't throw so this can be called from the
            // try block with the catch block calling abort() in case of an
            // exception.
            //
            invokeImpl(true); // userThread = true
        }

        override public Ice.AsyncCallback completed()
        {
            Debug.Assert(_is != null); // _is has been initialized prior to this call

            //
            // NOTE: this method is called from ConnectionI.parseMessage
            // with the connection locked. Therefore, it must not invoke
            // any user callbacks.
            //

            Debug.Assert(proxy_.ice_isTwoway()); // Can only be called for twoways.

            if(childObserver_ != null)
            {
                childObserver_.reply(_is.size() - Protocol.headerSize - 4);
                childObserver_.detach();
                childObserver_ = null;
            }

            byte replyStatus;
            try
            {
                replyStatus = _is.readByte();

                switch(replyStatus)
                {
                case ReplyStatus.replyOK:
                {
                    break;
                }

                case ReplyStatus.replyUserException:
                {
                    if(observer_ != null)
                    {
                        observer_.userException();
                    }
                    break;
                }

                case ReplyStatus.replyObjectNotExist:
                case ReplyStatus.replyFacetNotExist:
                case ReplyStatus.replyOperationNotExist:
                {
                    Ice.Identity id = new Ice.Identity();
                    id.read__(_is);

                    //
                    // For compatibility with the old FacetPath.
                    //
                    string[] facetPath = _is.readStringSeq();
                    string facet;
                    if(facetPath.Length > 0)
                    {
                        if(facetPath.Length > 1)
                        {
                            throw new Ice.MarshalException();
                        }
                        facet = facetPath[0];
                    }
                    else
                    {
                        facet = "";
                    }

                    string operation = _is.readString();

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

                    ex.id = id;
                    ex.facet = facet;
                    ex.operation = operation;
                    throw ex;
                }

                case ReplyStatus.replyUnknownException:
                case ReplyStatus.replyUnknownLocalException:
                case ReplyStatus.replyUnknownUserException:
                {
                    string unknown = _is.readString();

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

                    ex.unknown = unknown;
                    throw ex;
                }

                default:
                {
                    throw new Ice.UnknownReplyStatusException();
                }
                }

                return finished(replyStatus == ReplyStatus.replyOK);
            }
            catch(Ice.Exception ex)
            {
                return completed(ex);
            }
        }

        public Ice.OutputStream startWriteParams(Ice.FormatType format)
        {
            os_.startEncapsulation(_encoding, format);
            return os_;
        }

        public void endWriteParams()
        {
            os_.endEncapsulation();
        }

        public void writeEmptyParams()
        {
            os_.writeEmptyEncapsulation(_encoding);
        }

        public void writeParamEncaps(byte[] encaps)
        {
            if(encaps == null || encaps.Length == 0)
            {
                os_.writeEmptyEncapsulation(_encoding);
            }
            else
            {
                os_.writeEncapsulation(encaps);
            }
        }

        public Ice.InputStream startReadParams()
        {
            _is.startEncapsulation();
            return _is;
        }

        public void endReadParams()
        {
            _is.endEncapsulation();
        }

        public void readEmptyParams()
        {
            _is.skipEmptyEncapsulation();
        }

        public byte[] readParamEncaps()
        {
            Ice.EncodingVersion encoding;
            return _is.readEncapsulation(out encoding);
        }

        override public Ice.InputStream getIs()
        {
            // _is can already be initialized if the invocation is retried
            if(_is == null)
            {
                _is = new Ice.InputStream(instance_, Ice.Util.currentProtocolEncoding);
            }
            return _is;
        }

        public void throwUserException()
        {
            try
            {
                _is.startEncapsulation();
                _is.throwException(null);
            }
            catch(Ice.UserException)
            {
                _is.endEncapsulation();
                throw;
            }
        }

        public override void cacheMessageBuffers()
        {
            if(proxy_.reference__().getInstance().cacheMessageBuffers() > 0)
            {
                lock(this)
                {
                    if((state_ & StateCachedBuffers) > 0)
                    {
                        return;
                    }
                    state_ |= StateCachedBuffers;
                }

                if(_is != null)
                {
                    _is.reset();
                }
                os_.reset();

                proxy_.cacheMessageBuffers(_is, os_);

                _is = null;
                os_ = null;
            }
        }

        private Ice.EncodingVersion _encoding;
        private Ice.InputStream _is;

        //
        // If true this AMI request is being used for a generated synchronous invocation.
        //
        private bool _synchronous;

        private static Dictionary<string, string> _emptyContext = new Dictionary<string, string>();
    }

    public class CommunicatorFlushBatch : IceInternal.AsyncResultI
    {
        public static CommunicatorFlushBatch check(Ice.AsyncResult r, Ice.Communicator com, string operation)
        {
            if(r != null && r.getCommunicator() != com)
            {
                throw new System.ArgumentException("Communicator for call to end_" + operation +
                                                   " does not match communicator that was used to call " +
                                                   "corresponding begin_" + operation + " method");
            }
            return AsyncResultI.check<CommunicatorFlushBatch>(r, operation);
        }

        public CommunicatorFlushBatch(Ice.Communicator communicator, Instance instance, string op, object cookie) :
            base(communicator, instance, op, cookie)
        {

            observer_ = ObserverHelper.get(instance, op);

            //
            // _useCount is initialized to 1 to prevent premature callbacks.
            // The caller must invoke ready() after all flush requests have
            // been initiated.
            //
            _useCount = 1;
        }

        public void flushConnection(Ice.ConnectionI con)
        {
            lock(this)
            {
                ++_useCount;
            }

            try
            {
                Ice.AsyncCallback sentCB = null;
                FlushBatch flush = new FlushBatch(this);
                int batchRequestNum = con.getBatchRequestQueue().swap(flush.getOs());
                if(batchRequestNum == 0)
                {
                    flush.sent();
                }
                else
                {
                    con.sendAsyncRequest(flush, false, false, batchRequestNum, out sentCB);
                }
                Debug.Assert(sentCB == null);
            }
            catch(Ice.LocalException)
            {
                doCheck(false);
                throw;
            }
        }

        public void ready()
        {
            doCheck(true);
        }

        private void doCheck(bool userThread)
        {
            lock(this)
            {
                Debug.Assert(_useCount > 0);
                if(--_useCount > 0)
                {
                    return;
                }
            }

            Ice.AsyncCallback sentCB = sent(true);
            if(userThread)
            {
                sentSynchronously_ = true;
                if(sentCB != null)
                {
                    invokeSent(sentCB);
                }
            }
            else
            {
                if(sentCB != null)
                {
                    invokeSentAsync(sentCB);
                }
            }
        }

        class FlushBatch : OutgoingAsyncBase
        {
            public FlushBatch(CommunicatorFlushBatch outAsync) :
                base(outAsync.getCommunicator(), outAsync.instance_, outAsync.getOperation(), null)
            {
                _outAsync = outAsync;
            }

            public override Ice.AsyncCallback sent()
            {
                if(childObserver_ != null)
                {
                    childObserver_.detach();
                    childObserver_ = null;
                }
                _outAsync.doCheck(false);
                return null;
            }

            public override Ice.AsyncCallback completed(Ice.Exception ex)
            {
                if(childObserver_ != null)
                {
                    childObserver_.failed(ex.ice_id());
                    childObserver_.detach();
                    childObserver_ = null;
                }
                _outAsync.doCheck(false);
                return null;
            }

            protected override Ice.Instrumentation.InvocationObserver getObserver()
            {
                return _outAsync.getObserver();
            }

            private CommunicatorFlushBatch _outAsync;
        }
        private int _useCount;
    }


    public class ConnectionFlushBatch : OutgoingAsyncBase
    {
        public static ConnectionFlushBatch check(Ice.AsyncResult r, Ice.Connection con, string operation)
        {
            if(r != null && r.getConnection() != con)
            {
                throw new System.ArgumentException("Connection for call to end_" + operation +
                                                   " does not match connection that was used to call " +
                                                   "corresponding begin_" + operation + " method");
            }
            return AsyncResultI.check<ConnectionFlushBatch>(r, operation);
        }

        public ConnectionFlushBatch(Ice.ConnectionI con, Ice.Communicator communicator, Instance instance, string op,
                                    object cookie) :
            base(communicator, instance, op, cookie)
        {
            _connection = con;
        }

        public override Ice.Connection getConnection()
        {
            return _connection;
        }

        public void invoke()
        {
            try
            {
                int batchRequestNum = _connection.getBatchRequestQueue().swap(os_);

                bool isSent = false;
                Ice.AsyncCallback sentCB;
                if(batchRequestNum == 0)
                {
                    isSent = true;
                    sentCB = sent();
                }
                else
                {
                    isSent = _connection.sendAsyncRequest(this, false, false, batchRequestNum, out sentCB);
                }

                if(isSent)
                {
                    sentSynchronously_ = true;
                    if(sentCB != null)
                    {
                        invokeSent(sentCB);
                    }
                }
            }
            catch(RetryException ex)
            {
                Ice.AsyncCallback cb = completed(ex.get());
                if(cb != null)
                {
                    invokeCompletedAsync(cb);
                }
            }
            catch(Ice.Exception ex)
            {
                Ice.AsyncCallback cb = completed(ex);
                if(cb != null)
                {
                    invokeCompletedAsync(cb);
                }
            }
        }

        private Ice.ConnectionI _connection;
    }

    public class ProxyFlushBatch : ProxyOutgoingAsyncBase
    {
        public new static ProxyFlushBatch check(Ice.AsyncResult r, Ice.ObjectPrx prx, string operation)
        {
            return ProxyOutgoingAsyncBase.check<ProxyFlushBatch>(r, prx, operation);
        }

        public ProxyFlushBatch(Ice.ObjectPrxHelperBase prx, string operation, object cookie) :
            base(prx, operation, cookie)
        {
            observer_ = ObserverHelper.get(prx, operation);
            _batchRequestNum = prx.getBatchRequestQueue__().swap(os_);
        }

        public override bool invokeRemote(Ice.ConnectionI con, bool compress, bool resp, out Ice.AsyncCallback sentCB)
        {
            if(_batchRequestNum == 0)
            {
                sentCB = sent();
                return true;
            }
            cachedConnection_ = con;
            return con.sendAsyncRequest(this, compress, false, _batchRequestNum, out sentCB);
        }

        public override bool invokeCollocated(CollocatedRequestHandler handler, out Ice.AsyncCallback sentCB)
        {
            if(_batchRequestNum == 0)
            {
                sentCB = sent();
                return true;
            }
            return handler.invokeAsyncRequest(this, _batchRequestNum, false, out sentCB);
        }

        public void invoke()
        {
            Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(proxy_.reference__().getProtocol()));
            invokeImpl(true); // userThread = true
        }

        private int _batchRequestNum;
    }

    public class ProxyGetConnection : ProxyOutgoingAsyncBase, Ice.AsyncResult<Ice.Callback_Object_ice_getConnection>
    {
        public new static ProxyGetConnection check(Ice.AsyncResult r, Ice.ObjectPrx prx, string operation)
        {
            return ProxyOutgoingAsyncBase.check<ProxyGetConnection>(r, prx, operation);
        }

        public ProxyGetConnection(Ice.ObjectPrxHelperBase prx, string operation,
                                  ProxyTwowayCallback<Ice.Callback_Object_ice_getConnection> cb, object cookie) :
            base(prx, operation, cookie)
        {
            observer_ = ObserverHelper.get(prx, operation);
            _completed = cb;
        }

        public override bool invokeRemote(Ice.ConnectionI con, bool compress, bool resp, out Ice.AsyncCallback sentCB)
        {
            sentCB = null;
            cachedConnection_ = con;
            Ice.AsyncCallback cb = finished(true);
            if(cb != null)
            {
                invokeCompletedAsync(cb);
            }
            return true;
        }

        public override bool invokeCollocated(CollocatedRequestHandler handler, out Ice.AsyncCallback sentCB)
        {
            sentCB = null;
            Ice.AsyncCallback cb = finished(true);
            if(cb != null)
            {
                invokeCompletedAsync(cb);
            }
            return true;
        }

        public void invoke()
        {
            invokeImpl(true); // userThread = true
        }

        new public Ice.AsyncResult<Ice.Callback_Object_ice_getConnection> whenCompleted(Ice.ExceptionCallback excb)
        {
            base.whenCompleted(excb);
            return this;
        }

        virtual public Ice.AsyncResult<Ice.Callback_Object_ice_getConnection>
        whenCompleted(Ice.Callback_Object_ice_getConnection cb, Ice.ExceptionCallback excb)
        {
            if(cb == null && excb == null)
            {
                throw new System.ArgumentException("callback is null");
            }
            lock(this)
            {
                if(_responseCallback != null || exceptionCallback_ != null)
                {
                    throw new System.ArgumentException("callback already set");
                }
                _responseCallback = cb;
                exceptionCallback_ = excb;
            }
            setCompletedCallback(getCompletedCallback());
            return this;
        }

        new public Ice.AsyncResult<Ice.Callback_Object_ice_getConnection> whenSent(Ice.SentCallback cb)
        {
            base.whenSent(cb);
            return this;
        }

        protected override Ice.AsyncCallback getCompletedCallback()
        {
            return (Ice.AsyncResult result) => { _completed(this, _responseCallback, exceptionCallback_); };
        }

        private ProxyTwowayCallback<Ice.Callback_Object_ice_getConnection> _completed;
        private Ice.Callback_Object_ice_getConnection _responseCallback = null;
    }

    public abstract class OutgoingAsync<T> : OutgoingAsync, Ice.AsyncResult<T>
    {
        public OutgoingAsync(Ice.ObjectPrxHelperBase prx, string operation, object cookie) :
            base(prx, operation, cookie)
        {
        }

        public OutgoingAsync(Ice.ObjectPrxHelperBase prx, string operation, object cookie, Ice.InputStream iss,
                             Ice.OutputStream os) :
            base(prx, operation, cookie, iss, os)
        {
        }

        new public Ice.AsyncResult<T> whenCompleted(Ice.ExceptionCallback excb)
        {
            base.whenCompleted(excb);
            return this;
        }

        virtual public Ice.AsyncResult<T> whenCompleted(T cb, Ice.ExceptionCallback excb)
        {
            if(cb == null && excb == null)
            {
                throw new System.ArgumentException("callback is null");
            }
            lock(this)
            {
                if(responseCallback_ != null || exceptionCallback_ != null)
                {
                    throw new System.ArgumentException("callback already set");
                }
                responseCallback_ = cb;
                exceptionCallback_ = excb;
            }
            setCompletedCallback(getCompletedCallback());
            return this;
        }

        new public Ice.AsyncResult<T> whenSent(Ice.SentCallback cb)
        {
            base.whenSent(cb);
            return this;
        }

        protected T responseCallback_;
    }

    public class TwowayOutgoingAsync<T> : OutgoingAsync<T>
    {
        public TwowayOutgoingAsync(Ice.ObjectPrxHelperBase prx, string operation, ProxyTwowayCallback<T> cb,
                                   object cookie) :
            base(prx, operation, cookie)
        {
            Debug.Assert(cb != null);
            _completed = cb;
        }

        public TwowayOutgoingAsync(Ice.ObjectPrxHelperBase prx, string operation, ProxyTwowayCallback<T> cb,
                                   object cookie, Ice.InputStream iss, Ice.OutputStream os) :
            base(prx, operation, cookie, iss, os)
        {
            Debug.Assert(cb != null);
            _completed = cb;
        }

        override protected Ice.AsyncCallback getCompletedCallback()
        {
            return (Ice.AsyncResult result) => { _completed(this, responseCallback_, exceptionCallback_); };
        }

        private ProxyTwowayCallback<T> _completed;
    }

    public class OnewayOutgoingAsync<T> : OutgoingAsync<T>
    {
        public OnewayOutgoingAsync(Ice.ObjectPrxHelperBase prx, string operation, ProxyOnewayCallback<T> cb,
                                   object cookie) :
            base(prx, operation, cookie)
        {
            Debug.Assert(cb != null);
            _completed = cb;
        }

        public OnewayOutgoingAsync(Ice.ObjectPrxHelperBase prx, string operation, ProxyOnewayCallback<T> cb,
                                   object cookie, Ice.InputStream iss, Ice.OutputStream os) :
            base(prx, operation, cookie, iss, os)
        {
            Debug.Assert(cb != null);
            _completed = cb;
        }

        override protected Ice.AsyncCallback getCompletedCallback()
        {
            return (Ice.AsyncResult result) =>
            {
                try
                {
                    IceInternal.OutgoingAsync outAsync__ = (IceInternal.OutgoingAsync)result;
                    ((Ice.ObjectPrxHelperBase)(outAsync__.getProxy())).end__(outAsync__, outAsync__.getOperation());
                }
                catch(Ice.Exception ex__)
                {
                    if(exceptionCallback_ != null)
                    {
                        exceptionCallback_(ex__);
                    }
                    return;
                }
                _completed(responseCallback_);
            };
        }

        private ProxyOnewayCallback<T> _completed;
    }
}
